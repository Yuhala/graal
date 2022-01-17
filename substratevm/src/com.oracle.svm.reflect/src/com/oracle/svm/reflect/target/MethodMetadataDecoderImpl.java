/*
 * Copyright (c) 2021, 2021, Oracle and/or its affiliates. All rights reserved.
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
package com.oracle.svm.reflect.target;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.function.Supplier;

import org.graalvm.compiler.api.replacements.Fold;
import org.graalvm.compiler.core.common.util.UnsafeArrayTypeReader;

import com.oracle.svm.core.SubstrateUtil;
import com.oracle.svm.core.c.NonmovableArrays;
import com.oracle.svm.core.code.CodeInfo;
import com.oracle.svm.core.code.CodeInfoAccess;
import com.oracle.svm.core.code.CodeInfoTable;
import com.oracle.svm.core.hub.DynamicHub;
import com.oracle.svm.core.hub.Target_java_lang_reflect_RecordComponent;
import com.oracle.svm.core.reflect.MethodMetadataDecoder;
import com.oracle.svm.core.util.ByteArrayReader;
import org.graalvm.compiler.serviceprovider.JavaVersionUtil;

/**
 * The metadata for methods in the image is split into two arrays: one for the index and the other
 * for data. The index contains an array of integers pointing to offsets in the data, and indexed by
 * type ID. The data array contains arrays of method metadata, ordered by type ID, such that all
 * methods declared by a class are stored consecutively, in the following format:
 *
 * <pre>
 * {
 *     int queriedMethodsCount;
 *     ReflectMethodEncoding[] queriedMethods[queriedMethodsCount];
 *     int hidingMethodsCount;
 *     SimpleMethodEncoding[] hidingMethods[hidingMethodsCount];
 *     int declaringTypeIndex;             // index in frameInfoSourceClasses
 *     int reachableMethodsCount;
 *     SimpleMethodEncoding[] reachableMethods[reachableMethodsCount];
 * } TypeEncoding;
 * </pre>
 *
 * The declaring class is encoded before the reachable methods to avoid having to be decoded when
 * getting the queried and hiding methods, in which case the declaring class is available as an
 * argument and doesn't need to be retrieved from the encoding.
 *
 * The data for a queried method is stored in the following format:
 *
 * <pre>
 * {
 *     int methodNameIndex;                // index in frameInfoSourceMethodNames ("<init>" for constructors)
 *     int paramCount;
 *     int[] paramTypeIndices[paramCount]; // index in frameInfoSourceClasses
 *     int modifiers;
 *     int returnTypeIndex;                // index in frameInfoSourceClasses (void for constructors)
 *     int exceptionTypeCount;
 *     int[] exceptionTypeIndices[exceptionTypeCount]; // index in frameInfoSourceClasses
 *     int signatureIndex;                 // index in frameInfoSourceMethodNames
 *     int annotationsEncodingLength;
 *     byte[] annotationsEncoding[annotationsEncodingLength];
 *     int parameterAnnotationsEncodingLength;
 *     byte[] parameterAnnotationsEncoding[parameterAnnotationsEncodingLength];
 *     int typeAnnotationsEncodingLength;
 *     byte[] typeAnnotationsEncoding[typeAnnotationsEncodingLength];
 *     boolean hasRealParameterData;
 *     int reflectParameterCount;          // only if hasRealParameterData is true
 *     {
 *         int reflectParameterNameIndex;  // index in frameInfoSourceMethodNames
 *         int reflectParameterModifiers;
 *     } reflectParameters[reflectParameterCount];
 * } ReflectMethodEncoding;
 * </pre>
 *
 * The data for a hiding or reachable method is stored as follows:
 *
 * <pre>
 * {
 *     int methodNameIndex;                // index in frameInfoSourceMethodNames ("<init>" for constructors)
 *     int paramCount;
 *     int[] paramTypeIndices[paramCount]; // index in frameInfoSourceClasses
 * } SimpleMethodEncoding;
 * </pre>
 */
public class MethodMetadataDecoderImpl implements MethodMetadataDecoder {
    public static final int NO_METHOD_METADATA = -1;
    public static final int NULL_OBJECT = -1;
    public static final int COMPLETE_METHOD_FLAG = (1 << 31);

    @Fold
    static boolean hasQueriedMethods() {
        return true; // !ImageSingletons.lookup(RuntimeReflectionSupport.class).getQueriedOnlyMethods().isEmpty();
    }

    public Field[] parseFields(DynamicHub declaringType, byte[] encoding, boolean publicOnly) {
        UnsafeArrayTypeReader reader = UnsafeArrayTypeReader.create(encoding, 0, ByteArrayReader.supportsUnalignedMemoryAccess());
        CodeInfo codeInfo = CodeInfoTable.getImageCodeInfo();
        return decodeArray(reader, Field.class, () -> decodeField(reader, codeInfo, DynamicHub.toClass(declaringType), publicOnly));
    }

    public Method[] parseMethods(DynamicHub declaringType, byte[] encoding, boolean publicOnly) {
        UnsafeArrayTypeReader reader = UnsafeArrayTypeReader.create(encoding, 0, ByteArrayReader.supportsUnalignedMemoryAccess());
        CodeInfo codeInfo = CodeInfoTable.getImageCodeInfo();
        return decodeArray(reader, Method.class, () -> decodeMethod(reader, codeInfo, DynamicHub.toClass(declaringType), publicOnly));
    }

    public Constructor<?>[] parseConstructors(DynamicHub declaringType, byte[] encoding, boolean publicOnly) {
        UnsafeArrayTypeReader reader = UnsafeArrayTypeReader.create(encoding, 0, ByteArrayReader.supportsUnalignedMemoryAccess());
        CodeInfo codeInfo = CodeInfoTable.getImageCodeInfo();
        return decodeArray(reader, Constructor.class, () -> decodeConstructor(reader, codeInfo, DynamicHub.toClass(declaringType), publicOnly));
    }

    public Class<?>[] parseClasses(DynamicHub declaringType, byte[] encoding) {
        UnsafeArrayTypeReader reader = UnsafeArrayTypeReader.create(encoding, 0, ByteArrayReader.supportsUnalignedMemoryAccess());
        CodeInfo codeInfo = CodeInfoTable.getImageCodeInfo();
        return decodeArray(reader, Class.class, () -> decodeType(reader, codeInfo));
    }

    public Target_java_lang_reflect_RecordComponent[] parseRecordComponents(DynamicHub declaringType, byte[] encoding) {
        UnsafeArrayTypeReader reader = UnsafeArrayTypeReader.create(encoding, 0, ByteArrayReader.supportsUnalignedMemoryAccess());
        CodeInfo codeInfo = CodeInfoTable.getImageCodeInfo();
        return decodeArray(reader, Target_java_lang_reflect_RecordComponent.class, () -> decodeRecordComponent(reader, codeInfo, DynamicHub.toClass(declaringType)));
    }

    private static Field decodeField(UnsafeArrayTypeReader buf, CodeInfo info, Class<?> declaringClass, boolean publicOnly) {
        int modifiers = buf.getUVInt();
        if (publicOnly && !Modifier.isPublic(modifiers)) {
            return null;
        }
        String name = decodeName(buf, info);
        Class<?> type = decodeType(buf, info);
        boolean trustedFinal = buf.getU1() == 1;
        String signature = decodeName(buf, info);
        byte[] annotations = decodeByteArray(buf);
        byte[] typeAnnotations = decodeByteArray(buf);

        Target_java_lang_reflect_Field field = new Target_java_lang_reflect_Field();
        if (JavaVersionUtil.JAVA_SPEC >= 17) {
            field.constructorJDK17OrLater(declaringClass, name, type, modifiers, trustedFinal, -1, signature, annotations);
        } else {
            assert !trustedFinal;
            field.constructorJDK11OrEarlier(declaringClass, name, type, modifiers, -1, signature, annotations);
        }
        field.typeAnnotations = typeAnnotations;
        return SubstrateUtil.cast(field, Field.class);
    }

    private static Method decodeMethod(UnsafeArrayTypeReader buf, CodeInfo info, Class<?> declaringClass, boolean publicOnly) {
        int modifiers = buf.getUVInt();
        boolean complete = (modifiers & COMPLETE_METHOD_FLAG) != 0;
        if (!complete || publicOnly && !Modifier.isPublic(modifiers)) {
            return null;
        }
        String name = decodeName(buf, info);
        Class<?>[] parameterTypes = decodeArray(buf, Class.class, () -> decodeType(buf, info));
        Class<?> returnType = decodeType(buf, info);
        Class<?>[] exceptionTypes = decodeArray(buf, Class.class, () -> decodeType(buf, info));
        String signature = decodeName(buf, info);
        byte[] annotations = decodeByteArray(buf);
        byte[] parameterAnnotations = decodeByteArray(buf);
        byte[] typeAnnotations = decodeByteArray(buf);
        boolean hasRealParameterData = buf.getU1() == 1;
        ReflectParameterDescriptor[] reflectParameters = hasRealParameterData ? decodeArray(buf, ReflectParameterDescriptor.class, () -> decodeReflectParameter(buf, info)) : null;
        Object accessor = decodeObject(buf, info);

        Target_java_lang_reflect_Method method = new Target_java_lang_reflect_Method();
        method.constructor(declaringClass, name, parameterTypes, returnType, exceptionTypes, modifiers, -1, signature, annotations, parameterAnnotations, null);
        method.methodAccessor = (Target_jdk_internal_reflect_MethodAccessor) accessor;
        Target_java_lang_reflect_Executable executable = SubstrateUtil.cast(method, Target_java_lang_reflect_Executable.class);
        executable.hasRealParameterData = hasRealParameterData;
        if (hasRealParameterData) {
            fillReflectParameters(executable, reflectParameters);
        }
        executable.typeAnnotations = typeAnnotations;
        return SubstrateUtil.cast(method, Method.class);
    }

    private static Constructor<?> decodeConstructor(UnsafeArrayTypeReader buf, CodeInfo info, Class<?> declaringClass, boolean publicOnly) {
        int modifiers = buf.getUVInt();
        boolean complete = (modifiers >> 31) != 0;
        if (!complete || publicOnly && !Modifier.isPublic(modifiers)) {
            return null;
        }
        Class<?>[] parameterTypes = decodeArray(buf, Class.class, () -> decodeType(buf, info));
        Class<?>[] exceptionTypes = decodeArray(buf, Class.class, () -> decodeType(buf, info));
        String signature = decodeName(buf, info);
        byte[] annotations = decodeByteArray(buf);
        byte[] parameterAnnotations = decodeByteArray(buf);
        byte[] typeAnnotations = decodeByteArray(buf);
        boolean hasRealParameterData = buf.getU1() == 1;
        ReflectParameterDescriptor[] reflectParameters = hasRealParameterData ? decodeArray(buf, ReflectParameterDescriptor.class, () -> decodeReflectParameter(buf, info)) : null;
        Object accessor = decodeObject(buf, info);

        Target_java_lang_reflect_Constructor constructor = new Target_java_lang_reflect_Constructor();
        constructor.constructor(declaringClass, parameterTypes, exceptionTypes, modifiers, -1, signature, annotations, parameterAnnotations);
        constructor.constructorAccessor = (Target_jdk_internal_reflect_ConstructorAccessor) accessor;
        Target_java_lang_reflect_Executable executable = SubstrateUtil.cast(constructor, Target_java_lang_reflect_Executable.class);
        executable.hasRealParameterData = hasRealParameterData;
        if (hasRealParameterData) {
            fillReflectParameters(executable, reflectParameters);
        }
        executable.typeAnnotations = typeAnnotations;
        return SubstrateUtil.cast(constructor, Constructor.class);
    }

    private static void fillReflectParameters(Target_java_lang_reflect_Executable executable, ReflectParameterDescriptor[] reflectParameters) {
        executable.parameters = new Target_java_lang_reflect_Parameter[reflectParameters.length];
        for (int i = 0; i < reflectParameters.length; ++i) {
            executable.parameters[i] = new Target_java_lang_reflect_Parameter();
            executable.parameters[i].constructor(reflectParameters[i].getName(), reflectParameters[i].getModifiers(), executable, i);
        }
    }

    private static Target_java_lang_reflect_RecordComponent decodeRecordComponent(UnsafeArrayTypeReader buf, CodeInfo info, Class<?> declaringClass) {
        String name = decodeName(buf, info);
        Class<?> type = decodeType(buf, info);
        Method accessor = null; // TODO
        String signature = decodeName(buf, info);
        byte[] annotations = decodeByteArray(buf);
        byte[] typeAnnotations = decodeByteArray(buf);

        Target_java_lang_reflect_RecordComponent recordComponent = new Target_java_lang_reflect_RecordComponent();
        recordComponent.clazz = declaringClass;
        recordComponent.name = name;
        recordComponent.type = type;
        recordComponent.accessor = accessor;
        recordComponent.signature = signature;
        recordComponent.annotations = annotations;
        recordComponent.typeAnnotations = typeAnnotations;
        return recordComponent;
    }

    private static Class<?> decodeType(UnsafeArrayTypeReader buf, CodeInfo info) {
        int classIndex = buf.getSVInt();
        if (classIndex == NO_METHOD_METADATA) {
            return null;
        }
        return NonmovableArrays.getObject(CodeInfoAccess.getFrameInfoSourceClasses(info), classIndex);
    }

    private static String decodeName(UnsafeArrayTypeReader buf, CodeInfo info) {
        int nameIndex = buf.getSVInt();
        String name = NonmovableArrays.getObject(CodeInfoAccess.getFrameInfoSourceMethodNames(info), nameIndex);
        /* Interning the string to ensure JDK8 method search succeeds */
        return name == null ? null : name.intern();
    }

    private static Object decodeObject(UnsafeArrayTypeReader buf, CodeInfo info) {
        int objectIndex = buf.getSVInt();
        if (objectIndex == NULL_OBJECT) {
            return null;
        }
        return NonmovableArrays.getObject(CodeInfoAccess.getFrameInfoObjectConstants(info), objectIndex);
    }

    @SuppressWarnings("unchecked")
    private static <T> T[] decodeArray(UnsafeArrayTypeReader buf, Class<T> elementType, Supplier<T> elementDecoder) {
        int length = buf.getUVInt();
        T[] result = (T[]) Array.newInstance(elementType, length);
        for (int i = 0; i < length; ++i) {
            result[i] = elementDecoder.get();
        }
        return result;
    }

    private static byte[] decodeByteArray(UnsafeArrayTypeReader buf) {
        int length = buf.getUVInt();
        byte[] result = new byte[length];
        for (int i = 0; i < length; ++i) {
            result[i] = (byte) buf.getS1();
        }
        return result;
    }

    private static ReflectParameterDescriptor decodeReflectParameter(UnsafeArrayTypeReader buf, CodeInfo info) {
        String name = decodeName(buf, info);
        int modifiers = buf.getS4();
        return new ReflectParameterDescriptor(name, modifiers);
    }

    public static class ReflectParameterDescriptor {
        private final String name;
        private final int modifiers;

        public ReflectParameterDescriptor(String name, int modifiers) {
            this.name = name;
            this.modifiers = modifiers;
        }

        public String getName() {
            return name;
        }

        public int getModifiers() {
            return modifiers;
        }
    }
}
