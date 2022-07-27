/*
* 
* The MIT License (MIT)
* Copyright (c) 2022 Peterson Yuhala
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
package com.oracle.svm.core.graal.snippets;

import static org.graalvm.compiler.nodes.extended.BranchProbabilityNode.EXTREMELY_SLOW_PATH_PROBABILITY;
import static org.graalvm.compiler.nodes.extended.BranchProbabilityNode.probability;

import java.util.ListIterator;
import java.util.Map;
import java.util.function.Predicate;

import org.graalvm.compiler.api.replacements.Snippet;
import org.graalvm.compiler.api.replacements.Snippet.ConstantParameter;
import org.graalvm.compiler.api.replacements.SnippetReflectionProvider;
import org.graalvm.compiler.core.common.spi.ForeignCallDescriptor;
import org.graalvm.compiler.core.common.type.StampFactory;
import org.graalvm.compiler.graph.Node;
import org.graalvm.compiler.graph.Node.ConstantNodeParameter;
import org.graalvm.compiler.graph.Node.NodeIntrinsic;
import org.graalvm.compiler.graph.NodeClass;
import org.graalvm.compiler.graph.NodeMap;
import org.graalvm.compiler.nodeinfo.NodeCycles;
import org.graalvm.compiler.nodeinfo.NodeInfo;
import org.graalvm.compiler.nodeinfo.NodeSize;
import org.graalvm.compiler.nodes.FixedWithNextNode;
import org.graalvm.compiler.nodes.FrameState;
import org.graalvm.compiler.nodes.StructuredGraph;
import org.graalvm.compiler.nodes.UnreachableNode;
import org.graalvm.compiler.nodes.extended.ForeignCallNode;
import org.graalvm.compiler.nodes.spi.Lowerable;
import org.graalvm.compiler.nodes.spi.LoweringTool;
import org.graalvm.compiler.options.OptionValues;
import org.graalvm.compiler.phases.BasePhase;
import org.graalvm.compiler.phases.common.LoweringPhase;
import org.graalvm.compiler.phases.tiers.MidTierContext;
import org.graalvm.compiler.phases.tiers.Suites;
import org.graalvm.compiler.phases.util.Providers;
import org.graalvm.compiler.replacements.SnippetTemplate;
import org.graalvm.compiler.replacements.SnippetTemplate.Arguments;
import org.graalvm.compiler.replacements.SnippetTemplate.SnippetInfo;
import org.graalvm.compiler.replacements.Snippets;
import org.graalvm.nativeimage.ImageInfo;
import org.graalvm.nativeimage.ImageSingletons;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.word.UnsignedWord;
import org.graalvm.word.WordFactory;

import com.oracle.svm.core.annotate.AutomaticFeature;
import com.oracle.svm.core.annotate.RestrictHeapAccess;
import com.oracle.svm.core.annotate.RestrictHeapAccess.Access;
import com.oracle.svm.core.annotate.Uninterruptible;
import com.oracle.svm.core.code.CodeInfoAccess;
import com.oracle.svm.core.code.CodeInfoTable;
import com.oracle.svm.core.graal.GraalFeature;
import com.oracle.svm.core.graal.code.SubstrateBackend;
import com.oracle.svm.core.graal.meta.RuntimeConfiguration;
import com.oracle.svm.core.graal.meta.SubstrateForeignCallsProvider;
import com.oracle.svm.core.heap.Heap;
import com.oracle.svm.core.heap.RestrictHeapAccessCallees;
import com.oracle.svm.core.meta.SharedMethod;
import com.oracle.svm.core.snippets.ImplicitExceptions;
import com.oracle.svm.core.snippets.KnownIntrinsics;
import com.oracle.svm.core.snippets.SnippetRuntime;
import com.oracle.svm.core.snippets.SnippetRuntime.SubstrateForeignCallDescriptor;
import com.oracle.svm.core.snippets.SubstrateForeignCallTarget;
import com.oracle.svm.core.stack.StackOverflowCheck;
import com.oracle.svm.core.thread.JavaThreads;
import com.oracle.svm.core.thread.ThreadingSupportImpl;
import com.oracle.svm.core.thread.VMThreads;
import com.oracle.svm.core.threadlocal.FastThreadLocal;
import com.oracle.svm.core.threadlocal.FastThreadLocalFactory;
import com.oracle.svm.core.threadlocal.FastThreadLocalInt;
import com.oracle.svm.core.threadlocal.FastThreadLocalWord;
import com.oracle.svm.core.util.VMError;

import jdk.vm.ci.meta.ResolvedJavaMethod;

public final class SGXStackOverflowCheckImpl implements StackOverflowCheck {
    // The stack boundary for the stack overflow check
    public static final FastThreadLocalWord<UnsignedWord> stackBoundaryTL = FastThreadLocalFactory
            .createWord("StackOverflowCheckImpl.stackBoundaryTL").setMaxOffset(FastThreadLocal.FIRST_CACHE_LINE);

    /**
     * Stores a counter how often the yellow zone has been made available, so that
     * the yellow zone
     * is only protected after a matching number of calls. Note that the counter
     * doesn't start at 0:
     * 0 is the default value of thread local variables, so disallowing 0 as a valid
     * value allows us
     * to to detect error in the state transitions.
     */
    static final FastThreadLocalInt yellowZoneStateTL = FastThreadLocalFactory
            .createInt("StackOverflowCheckImpl.yellowZoneStateTL");
    static final int STATE_UNINITIALIZED = 0;
    static final int STATE_YELLOW_ENABLED = 1;

    public static final SubstrateForeignCallDescriptor THROW_CACHED_STACK_OVERFLOW_ERROR = SnippetRuntime
            .findForeignCall(SGXStackOverflowCheckImpl.class, "throwCachedStackOverflowError", true);
    public static final SubstrateForeignCallDescriptor THROW_NEW_STACK_OVERFLOW_ERROR = SnippetRuntime
            .findForeignCall(SGXStackOverflowCheckImpl.class, "throwNewStackOverflowError", true);

    static final SubstrateForeignCallDescriptor[] FOREIGN_CALLS = new SubstrateForeignCallDescriptor[] {
            THROW_CACHED_STACK_OVERFLOW_ERROR, THROW_NEW_STACK_OVERFLOW_ERROR };

    private static final StackOverflowError CACHED_STACK_OVERFLOW_ERROR = new StackOverflowError(
            ImplicitExceptions.NO_STACK_MSG);

    @Uninterruptible(reason = "Called while thread is being attached to the VM, i.e., when the thread state is not yet set up.")
    @Override
    public void initialize(IsolateThread thread) {
        // /*
        // * Get the real physical end of the stack. Everything past this point is
        // * memory-protected.
        // */
        // OSSupport osSupport =
        // ImageSingletons.lookup(StackOverflowCheck.OSSupport.class);
        // UnsignedWord stackBase = osSupport.lookupStackBase();
        // UnsignedWord stackEnd = osSupport.lookupStackEnd();

        // /* Initialize the stack base and the stack end thread locals. */
        // VMThreads.StackBase.set(thread, stackBase);
        // VMThreads.StackEnd.set(thread, stackEnd);

        // /*
        // * Set up our yellow and red zones. That memory is not memory protected, it is
        // a
        // * soft limit
        // * that we can change.
        // */
        // stackBoundaryTL.set(thread,
        // stackEnd.add(Options.StackYellowZoneSize.getValue() +
        // Options.StackRedZoneSize.getValue()));
        // yellowZoneStateTL.set(thread, STATE_YELLOW_ENABLED);
    }

    @Uninterruptible(reason = "Atomically manipulating state of multiple thread local variables.")
    @Override
    public void makeYellowZoneAvailable() {

    }

    @Override
    public boolean isYellowZoneAvailable() {
        return false;
    }

    @Uninterruptible(reason = "Atomically manipulating state of multiple thread local variables.")
    @Override
    public void protectYellowZone() {

    }

    @Override
    public int yellowAndRedZoneSize() {
        return 0;// Options.StackYellowZoneSize.getValue() + Options.StackRedZoneSize.getValue();
    }

    @Uninterruptible(reason = "Called by fatal error handling that is uninterruptible.")
    @Override
    public void disableStackOverflowChecksForFatalError() {

    }

    @Override
    public void updateStackOverflowBoundary() {

    }

    /**
     * Throw a cached {@link StackOverflowError} (without a stack trace) when we
     * know statically
     * that the method with the stack overflow check must never allocate.
     */
    @Uninterruptible(reason = "Must not have a stack overflow check: we are here because the stack overflow check failed.")
    @SubstrateForeignCallTarget(stubCallingConvention = true)
    private static void throwCachedStackOverflowError() {

    }

    /**
     * Throw a new {@link StackOverflowError} (with a stack trace). In cases where
     * we dynamically
     * find out that allocation is not possible at the current time, the cached
     * {@link StackOverflowError} is thrown.
     */
    @Uninterruptible(reason = "Must not have a stack overflow check: we are here because the stack overflow check failed.")
    @SubstrateForeignCallTarget(stubCallingConvention = true)
    private static void throwNewStackOverflowError() {

    }

    @Uninterruptible(reason = "Allow allocation now that yellow zone is available for new stack frames", calleeMustBe = false)
    @RestrictHeapAccess(reason = "Allow allocation now that yellow zone is available for new stack frames", access = Access.UNRESTRICTED)
    private static StackOverflowError newStackOverflowError() {
        /*
         * Now that the yellow zone is enabled, we can allocate the error and collect
         * the stack
         * trace. Note that this might even involve a GC run - the yellow zone is large
         * enough to
         * accommodate that.
         *
         * We need another method call indirection to make our uninterruptible and
         * no-allocation
         * verifiers happy.
         */
        return newStackOverflowError0();

    }

    private static StackOverflowError newStackOverflowError0() {
        return new StackOverflowError();
    }

    public static boolean needStackOverflowCheck(SharedMethod method) {
        if (Uninterruptible.Utils.isUninterruptible(method)) {
            /*
             * Uninterruptible methods are allowed to use the yellow and red zones of the
             * stack.
             * Also, the thread register and stack boundary might not be set up. We cannot
             * do a
             * stack overflow check.
             */
            return false;
        }
        return true;

    }

    public static long computeDeoptFrameSize(StructuredGraph graph) {
        return 0;
    }

    private static long computeDeoptFrameSize(FrameState state, NodeMap<Long> deoptFrameSizeCache) {
        return 0;
    }
}
