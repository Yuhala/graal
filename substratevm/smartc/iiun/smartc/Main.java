/*
 * Created on Fri Nov 05 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 * Testing polyglot native images
 */

package iiun.smartc;

import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.CurrentIsolate;

public class Main {

    // public static Context globalContext;
    public static void main(String[] args) {
        System.out.println("<<< ********** Hello Java! ******** >>>");

        // create context and run js snippet
        System.out.println("<<< ~~~~~~~~~~~~~~ Building context object! ~~~~~~~~~~~~~~~~ >>>");

        // Context ctx = Context.create();
        // context with all access
        Context ctx = Context.newBuilder().allowAllAccess(true).build();

        System.out.println("<<< ~~~~~~~~~~~~~~ Evaluationg js source code! ~~~~~~~~~~~~~~>>>");
        ctx.eval("js", "console.log('****** Hello javascript ******!');");
        ctx.eval("js", "console.log('****** Polyglot native image running in SGX enclave ******!');");

    }

    /**
     * Create context inside manual GraalVM entry point method.
     * This method can be called directly in C code. See sgx/Enclave/Enclave.cpp
     * file
     */
    @CEntryPoint(name = "enclave_create_context")
    public static void enclave_create_context(IsolateThread thread) {
        Context ctx = Context.newBuilder().allowAllAccess(true).build();
        System.out.println("<<< ~~~~~~~~~~~~~~~~ Hello Java! ~~~~~~~~~~~~~~~~~ >>>");

        // create context and run js snippet
        System.out.println("<<< ~~~~~~~~~~~~~~ Building context object! ~~~~~~~~~~~~~~~~ >>>");

        // Context ctx = Context.create();
        System.out.println("<<< !!!!!!!!!!!!!!!!! Created enclave context !!!!!!!!!!!!!!!!! >>>");
        ctx.eval("js", "console.log('******* Hello javascript ******!');");
    }

    /**
     * GraalVM entry point test
     */
    @CEntryPoint(name = "polytaint_add")
    public static int polytaint_add(IsolateThread thread, int a, int b) {
        return a + b;
    }

    /**
     * GC test.
     */
    @CEntryPoint(name = "gc_test")
    public static void gc_test(IsolateThread thread, int num) {
        System.out.println(">>>> in gc_test entry point");
        int sum = 0;
        for (int i = 0; i < num; i++) {
            sum += i;
        }
        System.out.println(">>>> Sum is: " + sum);
        System.gc();
    }

    public static void helloJava(int param) {
        System.out.println("----- Hello from Java -----::): param is: " + param);
    }

    public static void helloRuby(int param) {
        System.out.println("----- Hello from Ruby -----::): param is: " + param);
    }

    static void callJavaMethodFromJS() {
        System.out.println("Calling java method from JS context");
        Context ctx = Context.newBuilder().allowAllAccess(true).build();
        Value func1 = ctx.asValue(Main.class).getMember("static").getMember("helloJava");
        Value func2 = ctx.asValue(Main.class).getMember("static").getMember("helloRuby");

        // ctx.eval("js",
        // "function parent_func(m,p){func1 = m.func1;func2 =
        // m.func2;p1=p.param1;p2=p.param2; function
        // real_func(val1,val2){func1(111);func2(222);} real_func(p1,p2);}
        // parent_func;")
        // .execute(new MultiFunction(func1, func2), new Params(1, 2));
        // ctx.eval("js", "function test(f1,f2){f1(111);f2(222);}
        // test;").execute(func1,func2);
    }

    static void testEntryPoints() {
        // Testing centrypoints
        System.out.println("Creating isolate and Testing entrypoint");
        IsolateThread iso = CurrentIsolate.getCurrentThread();
        int sum = polytaint_add(iso, 23, 27);
        System.out.println("Sum from isolate entrypoint is: " + sum);
    }
}
