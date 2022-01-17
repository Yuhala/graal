/*
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
package com.oracle.svm.reflect.hosted;

import java.lang.annotation.Annotation;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Constructor;
import java.lang.reflect.Executable;
import java.lang.reflect.Field;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.lang.reflect.WildcardType;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Predicate;

import org.graalvm.compiler.debug.GraalError;
import org.graalvm.nativeimage.ImageSingletons;
import org.graalvm.nativeimage.hosted.Feature.DuringAnalysisAccess;
import org.graalvm.nativeimage.impl.ConfigurationCondition;
import org.graalvm.nativeimage.impl.RuntimeReflectionSupport;
import org.graalvm.util.GuardedAnnotationAccess;

import com.oracle.graal.pointsto.constraints.UnsupportedFeatureException;
import com.oracle.graal.pointsto.meta.AnalysisField;
import com.oracle.graal.pointsto.meta.AnalysisMethod;
import com.oracle.graal.pointsto.meta.AnalysisType;
import com.oracle.svm.core.SubstrateOptions;
import com.oracle.svm.core.hub.AnnotationTypeSupport;
import com.oracle.svm.core.hub.ClassForNameSupport;
import com.oracle.svm.core.hub.DynamicHub;
import com.oracle.svm.core.jdk.RecordSupport;
import com.oracle.svm.core.jdk.SealedClassSupport;
import com.oracle.svm.core.jdk.proxy.DynamicProxyRegistry;
import com.oracle.svm.core.util.UserError;
import com.oracle.svm.core.util.VMError;
import com.oracle.svm.hosted.ConditionalConfigurationRegistry;
import com.oracle.svm.hosted.FeatureImpl.DuringAnalysisAccessImpl;
import com.oracle.svm.hosted.FeatureImpl.FeatureAccessImpl;
import com.oracle.svm.hosted.annotation.AnnotationSubstitutionType;
import com.oracle.svm.hosted.substitute.SubstitutionReflectivityFilter;
import com.oracle.svm.util.ReflectionUtil;

import jdk.vm.ci.meta.JavaType;
import jdk.vm.ci.meta.ResolvedJavaMethod;
import jdk.vm.ci.meta.ResolvedJavaType;
import sun.reflect.annotation.AnnotationType;
import sun.reflect.annotation.TypeAnnotation;
import sun.reflect.annotation.TypeAnnotationParser;

public class ReflectionDataBuilder extends ConditionalConfigurationRegistry implements RuntimeReflectionSupport {

    public static final Field[] EMPTY_FIELDS = new Field[0];
    public static final Method[] EMPTY_METHODS = new Method[0];

    private final Set<Class<?>> modifiedClasses = ConcurrentHashMap.newKeySet();
    private boolean sealed;

    private final Set<Class<?>> reflectionClasses = Collections.newSetFromMap(new ConcurrentHashMap<>());
    private final Set<Executable> reflectionMethods = ConcurrentHashMap.newKeySet();
    private final Map<Executable, Object> methodAccessors = new ConcurrentHashMap<>();
    private final Set<Field> reflectionFields = Collections.newSetFromMap(new ConcurrentHashMap<>());
    private final Set<Executable> queriedMethods = ConcurrentHashMap.newKeySet();
    private final Set<AnalysisMethod> hidingMethods = ConcurrentHashMap.newKeySet();
    private final Set<Executable> registeredMethods = ConcurrentHashMap.newKeySet();
    private final Set<Field> registeredFields = ConcurrentHashMap.newKeySet();

    private final Set<Class<?>> processedClasses = new HashSet<>();

    /* Keep track of annotation interface members to include in proxy classes */
    private final Map<Class<?>, Set<Member>> annotationMembers = new HashMap<>();

    private final ReflectionDataAccessors accessors;

    public ReflectionDataBuilder(FeatureAccessImpl access) {
        accessors = new ReflectionDataAccessors(access);
    }

    @Override
    public void register(ConfigurationCondition condition, Class<?>... classes) {
        checkNotSealed();
        registerConditionalConfiguration(condition, () -> registerClasses(classes));
    }

    private void registerClasses(Class<?>[] classes) {
        for (Class<?> clazz : classes) {
            if (reflectionClasses.add(clazz)) {
                modifiedClasses.add(clazz);
            }
        }
    }

    @Override
    public void register(ConfigurationCondition condition, boolean queriedOnly, Executable... methods) {
        checkNotSealed();
        registerConditionalConfiguration(condition, () -> registerMethods(queriedOnly, methods));
    }

    private void registerMethods(boolean queriedOnly, Executable[] methods) {
        for (Executable method : methods) {
            boolean added = queriedOnly ? queriedMethods.add(method) : reflectionMethods.add(method);
            if (added) {
                modifiedClasses.add(method.getDeclaringClass());
            }
        }
    }

    @Override
    public void register(ConfigurationCondition condition, boolean finalIsWritable, Field... fields) {
        checkNotSealed();
        registerConditionalConfiguration(condition, () -> registerFields(fields));
    }

    private void registerFields(Field[] fields) {
        // Unsafe and write accesses are always enabled for fields because accessors use Unsafe.
        for (Field field : fields) {
            if (reflectionFields.add(field)) {
                modifiedClasses.add(field.getDeclaringClass());
            }
        }
    }

    private void checkNotSealed() {
        if (sealed) {
            throw UserError.abort("Too late to add classes, methods, and fields for reflective access. Registration must happen in a Feature before the analysis has finished.");
        }
    }

    protected void duringAnalysis(DuringAnalysisAccess a) {
        DuringAnalysisAccessImpl access = (DuringAnalysisAccessImpl) a;
        processReachableTypes(access);
        processRegisteredElements(access);
        processMethodMetadata(access);
    }

    /*
     * Process all reachable types, looking for array types or types that have an enclosing method
     * or constructor. Initialize the reflection metadata for those types.
     */
    private void processReachableTypes(DuringAnalysisAccessImpl access) {
        /*
         * We need to find all classes that have an enclosingMethod or enclosingConstructor.
         * Unfortunately, there is no reverse lookup (ask a Method or Constructor about the classes
         * they contain), so we need to iterate through all types that have been loaded so far.
         * Accessing the original java.lang.Class for a ResolvedJavaType is not 100% reliable,
         * especially in the case of class and method substitutions. But it is the best we can do
         * here, and we assume that user code that requires reflection support is not using
         * substitutions.
         */
        for (AnalysisType type : access.getUniverse().getTypes()) {
            Class<?> originalClass = type.getJavaClass();
            if (originalClass != null) {
                if (processedClasses.contains(originalClass)) {
                    /* Class has already been processed. */
                    continue;
                }
                if (type.isArray() && !access.isReachable(type)) {
                    /*
                     * We don't want the array type (and its elemental type) to become reachable as
                     * a result of initializing its reflection data.
                     */
                    continue;
                }
                if (type.isArray() || enclosingMethodOrConstructor(originalClass) != null) {
                    /*
                     * This type is either an array or it has an enclosing method or constructor. In
                     * either case we process the class, i.e., initialize its reflection data, mark
                     * it as processed and require an analysis iteration.
                     */
                    processClass(access, originalClass);
                    processedClasses.add(originalClass);
                    access.requireAnalysisIteration();
                }
                if (type.getWrappedWithoutResolve() instanceof AnnotationSubstitutionType) {
                    /*
                     * Proxy classes for annotations present the annotation default methods and
                     * fields as their own.
                     */
                    ResolvedJavaType annotationType = ((AnnotationSubstitutionType) type.getWrappedWithoutResolve()).getAnnotationInterfaceType();
                    Class<?> annotationClass = access.getUniverse().lookup(annotationType).getJavaClass();
                    if (!annotationMembers.containsKey(annotationClass)) {
                        processClass(access, annotationClass);
                    }
                    for (Member member : annotationMembers.get(annotationClass)) {
                        try {
                            if (member instanceof Field) {
                                Field field = (Field) member;
                                register(ConfigurationCondition.alwaysTrue(), false, originalClass.getDeclaredField(field.getName()));
                            } else if (member instanceof Method) {
                                Method method = (Method) member;
                                register(ConfigurationCondition.alwaysTrue(), false, originalClass.getDeclaredMethod(method.getName(), method.getParameterTypes()));
                            }
                        } catch (NoSuchFieldException | NoSuchMethodException e) {
                            /*
                             * The annotation member is not present in the proxy class so we don't
                             * add it.
                             */
                        }
                    }
                }
            }
        }
    }

    /**
     * See {@link MethodMetadataEncoderImpl} for details.
     */
    protected void processMethodMetadata(DuringAnalysisAccessImpl access) {
        for (Field reflectField : reflectionFields) {
            if (!registeredFields.contains(reflectField) && !SubstitutionReflectivityFilter.shouldExclude(reflectField, access.getMetaAccess(), access.getUniverse())) {
                AnalysisField analysisField = access.getMetaAccess().lookupJavaField(reflectField);
                registerTypesForField(access, analysisField, reflectField);
                registeredFields.add(reflectField);
            }
        }
        for (Executable reflectMethod : queriedMethods) {
            if (!registeredMethods.contains(reflectMethod) && !SubstitutionReflectivityFilter.shouldExclude(reflectMethod, access.getMetaAccess(), access.getUniverse()) &&
                            !reflectionMethods.contains(reflectMethod)) {
                AnalysisMethod analysisMethod = access.getMetaAccess().lookupJavaMethod(reflectMethod);
                registerTypesForMethod(access, analysisMethod, reflectMethod);
                registerHidingSubTypeMethods(analysisMethod, analysisMethod.getDeclaringClass());
                registeredMethods.add(reflectMethod);
            }
        }
        for (Executable method : reflectionMethods) {
            if (!registeredMethods.contains(method) && !SubstitutionReflectivityFilter.shouldExclude(method, access.getMetaAccess(), access.getUniverse())) {
                AnalysisMethod analysisMethod = access.getMetaAccess().lookupJavaMethod(method);
                registerTypesForMethod(access, analysisMethod, method);
                registerHidingSubTypeMethods(analysisMethod, analysisMethod.getDeclaringClass());
                methodAccessors.putIfAbsent(method, ImageSingletons.lookup(ReflectionFeature.class).getOrCreateAccessor(method));
                registeredMethods.add(method);
            }
        }
        if (SubstrateOptions.IncludeMethodData.getValue()) {
            for (AnalysisMethod method : access.getUniverse().getMethods()) {
                if (method.isReachable() && !method.isIntrinsicMethod()) {
                    registerTypesForReachableMethod(access, method);
                }
            }
        }
    }

    private final Map<AnalysisMethod, Set<AnalysisType>> seenHidingMethods = new HashMap<>();

    private void registerHidingSubTypeMethods(AnalysisMethod method, AnalysisType type) {
        if (!type.equals(method.getDeclaringClass()) && type.isReachable()) {
            if (!seenHidingMethods.containsKey(method) || !seenHidingMethods.get(method).contains(type)) {
                seenHidingMethods.computeIfAbsent(method, m -> new HashSet<>()).add(type);
                try {
                    /*
                     * Using findMethod here which uses getDeclaredMethods internally, instead of
                     * resolveConcreteMethods which gives different results in at least two
                     * scenarios:
                     *
                     * 1) When resolving a static method, resolveConcreteMethods does not return a
                     * subclass method with the same signature, since they are actually fully
                     * distinct methods. However these methods need to be included in the hiding
                     * list because them showing up in a reflection query would be wrong.
                     *
                     * 2) When resolving an interface method from an abstract class,
                     * resolveConcreteMethods returns an undeclared method with the abstract
                     * subclass as declaring class, which is not the reflection API behavior.
                     */
                    AnalysisMethod subClassMethod = type.findMethod(method.getName(), method.getSignature());
                    if (subClassMethod != null) {
                        hidingMethods.add(subClassMethod);
                    }
                } catch (UnsupportedFeatureException | LinkageError e) {
                    /*
                     * A method that is not supposed to end up in the image is considered as being
                     * absent for reflection purposes.
                     */
                }
            }
        }
        for (AnalysisType subType : type.getSubTypes()) {
            if (!subType.equals(type)) {
                registerHidingSubTypeMethods(method, subType);
            }
        }
    }

    private static final Method parseAllTypeAnnotations = ReflectionUtil.lookupMethod(TypeAnnotationParser.class, "parseAllTypeAnnotations", AnnotatedElement.class);

    private static void registerTypesForField(DuringAnalysisAccessImpl access, AnalysisField analysisField, Field reflectField) {
        makeAnalysisTypeReachable(access, analysisField.getDeclaringClass());
        makeAnalysisTypeReachable(access, analysisField.getType());
        makeTypeReachable(access, reflectField.getGenericType());

        /*
         * Enable runtime instantiation of annotations
         */
        for (Annotation annotation : GuardedAnnotationAccess.getDeclaredAnnotations(analysisField)) {
            registerTypesForAnnotationValue(access, annotation.annotationType(), annotation);
        }
        try {
            for (TypeAnnotation typeAnnotation : (TypeAnnotation[]) parseAllTypeAnnotations.invoke(null, reflectField)) {
                // Checkstyle: allow direct annotation access
                registerTypesForAnnotationValue(access, typeAnnotation.getAnnotation().annotationType(), typeAnnotation.getAnnotation());
                // Checkstyle: disallow direct annotation access
            }
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw GraalError.shouldNotReachHere();
        }
    }

    private static void registerTypesForMethod(DuringAnalysisAccessImpl access, AnalysisMethod analysisMethod, Executable reflectMethod) {
        makeAnalysisTypeReachable(access, analysisMethod.getDeclaringClass());

        for (TypeVariable<?> type : reflectMethod.getTypeParameters()) {
            makeTypeReachable(access, type);
        }
        for (Type paramType : analysisMethod.getGenericParameterTypes()) {
            makeTypeReachable(access, paramType);
        }
        if (!analysisMethod.isConstructor()) {
            makeTypeReachable(access, ((Method) reflectMethod).getGenericReturnType());
        }
        for (Type exceptionType : reflectMethod.getGenericExceptionTypes()) {
            makeTypeReachable(access, exceptionType);
        }

        /*
         * Enable runtime instantiation of annotations
         */
        for (Annotation annotation : GuardedAnnotationAccess.getDeclaredAnnotations(analysisMethod)) {
            registerTypesForAnnotationValue(access, annotation.annotationType(), annotation);
        }
        for (Annotation[] parameterAnnotations : reflectMethod.getParameterAnnotations()) {
            for (Annotation parameterAnnotation : parameterAnnotations) {
                registerTypesForAnnotationValue(access, parameterAnnotation.annotationType(), parameterAnnotation);
            }
        }
        try {
            for (TypeAnnotation typeAnnotation : (TypeAnnotation[]) parseAllTypeAnnotations.invoke(null, reflectMethod)) {
                // Checkstyle: allow direct annotation access
                registerTypesForAnnotationValue(access, typeAnnotation.getAnnotation().annotationType(), typeAnnotation.getAnnotation());
                // Checkstyle: disallow direct annotation access
            }
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw GraalError.shouldNotReachHere();
        }
    }

    private static void registerTypesForReachableMethod(DuringAnalysisAccessImpl access, AnalysisMethod analysisMethod) {
        makeAnalysisTypeReachable(access, analysisMethod.getDeclaringClass());
        for (JavaType paramType : analysisMethod.toParameterTypes()) {
            makeAnalysisTypeReachable(access, (AnalysisType) paramType);
        }
    }

    private static final Set<Type> seenTypes = new HashSet<>();

    @SuppressWarnings("unchecked")
    private static void makeTypeReachable(DuringAnalysisAccessImpl access, Type type) {
        if (type == null || seenTypes.contains(type)) {
            return;
        }
        seenTypes.add(type);
        if (type instanceof Class<?> && !SubstitutionReflectivityFilter.shouldExclude((Class<?>) type, access.getMetaAccess(), access.getUniverse())) {
            Class<?> clazz = (Class<?>) type;
            makeAnalysisTypeReachable(access, access.getMetaAccess().lookupJavaType(clazz));

            /*
             * Reflection signature parsing will try to instantiate classes via Class.forName().
             */
            if (ClassForNameSupport.forNameOrNull(clazz.getName(), null) == null) {
                access.requireAnalysisIteration();
            }
            ClassForNameSupport.registerClass(clazz);
        } else if (type instanceof TypeVariable<?>) {
            for (Type bound : ((TypeVariable<?>) type).getBounds()) {
                makeTypeReachable(access, bound);
            }
        } else if (type instanceof GenericArrayType) {
            makeTypeReachable(access, ((GenericArrayType) type).getGenericComponentType());
        } else if (type instanceof ParameterizedType) {
            ParameterizedType parameterizedType = (ParameterizedType) type;
            for (Type actualType : parameterizedType.getActualTypeArguments()) {
                makeTypeReachable(access, actualType);
            }
            makeTypeReachable(access, parameterizedType.getRawType());
            makeTypeReachable(access, parameterizedType.getOwnerType());
        } else if (type instanceof WildcardType) {
            WildcardType wildcardType = (WildcardType) type;
            for (Type lowerBound : wildcardType.getLowerBounds()) {
                makeTypeReachable(access, lowerBound);
            }
            for (Type upperBound : wildcardType.getUpperBounds()) {
                makeTypeReachable(access, upperBound);
            }
        }
    }

    @SuppressWarnings("unchecked")
    private static void registerTypesForAnnotationValue(DuringAnalysisAccessImpl access, Class<?> type, Object value) {
        if (type.isAnnotation()) {
            makeAnalysisTypeReachable(access, access.getMetaAccess().lookupJavaType(type));
            /*
             * Parsing annotation data in reflection classes requires being able to instantiate all
             * annotation types at runtime.
             */
            ImageSingletons.lookup(AnnotationTypeSupport.class).createInstance((Class<? extends Annotation>) type);
            ImageSingletons.lookup(DynamicProxyRegistry.class).addProxyClass(type);

            Annotation annotation = (Annotation) value;
            AnnotationType annotationType = AnnotationType.getInstance((Class<? extends Annotation>) type);
            for (Map.Entry<String, Class<?>> entry : annotationType.memberTypes().entrySet()) {
                String valueName = entry.getKey();
                Class<?> valueType = entry.getValue();
                try {
                    Method getAnnotationValue = annotationType.members().get(valueName);
                    getAnnotationValue.setAccessible(true);
                    Object annotationValue = getAnnotationValue.invoke(annotation);
                    registerTypesForAnnotationValue(access, valueType, annotationValue);
                } catch (IllegalAccessException | InvocationTargetException e) {
                    throw GraalError.shouldNotReachHere();
                }
            }
        } else if (type.isArray()) {
            Class<?> componentType = type.getComponentType();
            if (!componentType.isPrimitive()) {
                for (Object val : (Object[]) value) {
                    registerTypesForAnnotationValue(access, componentType, val);
                }
            }
        } else if (type == Class.class) {
            makeAnalysisTypeReachable(access, access.getMetaAccess().lookupJavaType((Class<?>) value));
        } else if (type.isEnum()) {
            makeAnalysisTypeReachable(access, access.getMetaAccess().lookupJavaType(type));
        }
    }

    private static void makeAnalysisTypeReachable(DuringAnalysisAccessImpl access, AnalysisType type) {
        if (type.registerAsReachable()) {
            access.requireAnalysisIteration();
        }
    }

    private void processRegisteredElements(DuringAnalysisAccessImpl access) {
        if (modifiedClasses.isEmpty()) {
            return;
        }
        access.requireAnalysisIteration();

        for (Class<?> clazz : modifiedClasses) {
            processClass(access, clazz);
        }
        modifiedClasses.clear();
    }

    private void processClass(DuringAnalysisAccessImpl access, Class<?> clazz) {
        if (SubstitutionReflectivityFilter.shouldExclude(clazz, access.getMetaAccess(), access.getUniverse())) {
            return;
        }

        AnalysisType type = access.getMetaAccess().lookupJavaType(clazz);
        /*
         * Make sure the class is registered as reachable before its fields are accessed below to
         * build the reflection metadata.
         */
        type.registerAsReachable();
        DynamicHub hub = access.getHostVM().dynamicHub(type);

        if (reflectionClasses.contains(clazz)) {
            ClassForNameSupport.registerClass(clazz);
        }

        if (type.isAnnotation()) {
            /*
             * Cache the annotation members to allow proxy classes seen later to include those in
             * their own reflection data
             */
            Object originalReflectionData = accessors.getReflectionData(clazz);
            Set<Member> members = new HashSet<>();
            Collections.addAll(members, filterFields(accessors.getDeclaredFields(originalReflectionData), reflectionFields, access));
            Collections.addAll(members, filterMethods(accessors.getDeclaredMethods(originalReflectionData), reflectionMethods, access));
            annotationMembers.put(clazz, members);
            access.requireAnalysisIteration(); /* Need the proxy class to see the added members */
        }
    }

    private Object[] buildRecordComponents(Class<?> clazz, DuringAnalysisAccessImpl access) {
        RecordSupport support = RecordSupport.singleton();
        if (!support.isRecord(clazz)) {
            return null;
        }

        /*
         * RecordComponent objects expose the "accessor method" as a java.lang.reflect.Method
         * object. We leverage this tight coupling of RecordComponent and its accessor method to
         * avoid a separate reflection configuration for record components: When all accessor
         * methods of the record class are registered for reflection, then the record components are
         * available. We do not want to expose a partial list of record components, that would be
         * confusing and error-prone. So as soon as a single accessor method is missing from the
         * reflection configuration, we provide no record components. Accessing the record
         * components in that case will throw an exception at image run time, see
         * DynamicHub.getRecordComponents0().
         */
        Method[] allMethods = support.getRecordComponentAccessorMethods(clazz);
        Method[] filteredMethods = filterMethods(allMethods, reflectionMethods, access);

        if (allMethods.length == filteredMethods.length) {
            return support.getRecordComponents(clazz);
        } else {
            return null;
        }
    }

    protected void afterAnalysis() {
        sealed = true;
        if (!modifiedClasses.isEmpty()) {
            throw UserError.abort("Registration of classes, methods, and fields for reflective access during analysis must set DuringAnalysisAccess.requireAnalysisIteration().");
        }
    }

    private Executable enclosingMethodOrConstructor(Class<?> clazz) {
        Method enclosingMethod;
        Constructor<?> enclosingConstructor;
        try {
            enclosingMethod = clazz.getEnclosingMethod();
            enclosingConstructor = clazz.getEnclosingConstructor();
        } catch (TypeNotPresentException | LinkageError e) {
            /*
             * If any of the methods or fields in the class of the enclosing method reference
             * missing types or types that have incompatible changes a LinkageError is thrown. Skip
             * the class.
             */
            return null;
        } catch (InternalError ex) {
            /*
             * Could not find the enclosing method of the class. This is a host VM error which can
             * happen due to invalid bytecode. For example if the eclosing method index points to a
             * synthetic method for a anonymous class declared inside a lambda. We skip registering
             * the enclosing method for such classes.
             */
            return null;
        }

        if (enclosingMethod == null && enclosingConstructor == null) {
            return null;
        }
        if (enclosingMethod != null && enclosingConstructor != null) {
            throw VMError.shouldNotReachHere("Class has both an enclosingMethod and an enclosingConstructor: " + clazz + ", " + enclosingMethod + ", " + enclosingConstructor);
        }

        Executable enclosingMethodOrConstructor = enclosingMethod != null ? enclosingMethod : enclosingConstructor;

        if (reflectionMethods.contains(enclosingMethodOrConstructor)) {
            return enclosingMethodOrConstructor;
        } else {
            return null;
        }
    }

    private static Field[] filterFields(Object fields, Set<Field> filterSet, DuringAnalysisAccessImpl access) {
        return filterFields(fields, filterSet::contains, access);
    }

    private static Field[] filterFields(Object fields, Predicate<Field> filter, DuringAnalysisAccessImpl access) {
        if (fields == null) {
            return EMPTY_FIELDS;
        }
        List<Field> result = new ArrayList<>();
        for (Field field : (Field[]) fields) {
            if (filter.test(field) && !SubstitutionReflectivityFilter.shouldExclude(field, access.getMetaAccess(), access.getUniverse())) {
                result.add(field);
            }
        }
        return result.toArray(EMPTY_FIELDS);
    }

    private static Method[] filterMethods(Object methods, Set<Executable> filter, DuringAnalysisAccessImpl access) {
        return filterMethods(methods, filter, access, EMPTY_METHODS);
    }

    @SuppressWarnings("unchecked")
    private static <T extends Executable> T[] filterMethods(Object methods, Set<Executable> filter, DuringAnalysisAccessImpl access, T[] emptyArray) {
        if (methods == null) {
            return emptyArray;
        }
        List<T> result = new ArrayList<>();
        for (T method : (T[]) methods) {
            if (filter.contains(method) && !SubstitutionReflectivityFilter.shouldExclude(method, access.getMetaAccess(), access.getUniverse())) {
                result.add(method);
            }
        }
        return result.toArray(emptyArray);
    }

    @Override
    public Set<Field> getReflectionFields() {
        return Collections.unmodifiableSet(registeredFields);
    }

    @Override
    public Set<Executable> getReflectionExecutables() {
        return Collections.unmodifiableSet(registeredMethods);
    }

    @Override
    public Object getAccessor(Executable method) {
        return methodAccessors.get(method);
    }

    @Override
    public Set<ResolvedJavaMethod> getHidingReflectionMethods() {
        return Collections.unmodifiableSet(hidingMethods);
    }

    @Override
    public int getReflectionClassesCount() {
        return reflectionClasses.size();
    }

    @Override
    public int getReflectionMethodsCount() {
        return reflectionMethods.size();
    }

    @Override
    public int getReflectionFieldsCount() {
        return reflectionFields.size();
    }

    static final class ReflectionDataAccessors {
        private final Method reflectionDataMethod;
        private final Field declaredFieldsField;
        private final Field publicFieldsField;
        private final Field declaredMethodsField;
        private final Field publicMethodsField;
        private final Field declaredConstructorsField;
        private final Field publicConstructorsField;
        private final Field declaredPublicFieldsField;
        private final Field declaredPublicMethodsField;

        ReflectionDataAccessors(FeatureAccessImpl access) {
            reflectionDataMethod = ReflectionUtil.lookupMethod(Class.class, "reflectionData");
            Class<?> originalReflectionDataClass = access.getImageClassLoader().findClassOrFail("java.lang.Class$ReflectionData");
            declaredFieldsField = ReflectionUtil.lookupField(originalReflectionDataClass, "declaredFields");
            publicFieldsField = ReflectionUtil.lookupField(originalReflectionDataClass, "publicFields");
            declaredMethodsField = ReflectionUtil.lookupField(originalReflectionDataClass, "declaredMethods");
            publicMethodsField = ReflectionUtil.lookupField(originalReflectionDataClass, "publicMethods");
            declaredConstructorsField = ReflectionUtil.lookupField(originalReflectionDataClass, "declaredConstructors");
            publicConstructorsField = ReflectionUtil.lookupField(originalReflectionDataClass, "publicConstructors");
            declaredPublicFieldsField = ReflectionUtil.lookupField(originalReflectionDataClass, "declaredPublicFields");
            declaredPublicMethodsField = ReflectionUtil.lookupField(originalReflectionDataClass, "declaredPublicMethods");
        }

        public Object getReflectionData(Class<?> clazz) {
            try {
                return reflectionDataMethod.invoke(clazz);
            } catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getDeclaredFields(Object obj) {
            try {
                return declaredFieldsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getPublicFields(Object obj) {
            try {
                return publicFieldsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getDeclaredMethods(Object obj) {
            try {
                return declaredMethodsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getPublicMethods(Object obj) {
            try {
                return publicMethodsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getDeclaredConstructors(Object obj) {
            try {
                return declaredConstructorsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getPublicConstructors(Object obj) {
            try {
                return publicConstructorsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getDeclaredPublicFields(Object obj) {
            try {
                return declaredPublicFieldsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }

        public Object getDeclaredPublicMethods(Object obj) {
            try {
                return declaredPublicMethodsField.get(obj);
            } catch (IllegalAccessException e) {
                throw VMError.shouldNotReachHere(e);
            }
        }
    }
}
