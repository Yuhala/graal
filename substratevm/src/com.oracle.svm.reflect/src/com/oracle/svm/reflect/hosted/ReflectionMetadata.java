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
package com.oracle.svm.reflect.hosted;

// Checkstyle: allow reflection

import java.lang.annotation.Annotation;

import org.graalvm.nativeimage.ImageSingletons;
import org.graalvm.nativeimage.hosted.Feature;

import com.oracle.svm.core.annotate.AutomaticFeature;
import com.oracle.svm.core.reflect.MethodMetadataDecoder;
import com.oracle.svm.hosted.image.NativeImageCodeCache.MethodMetadataEncoderFactory;
import com.oracle.svm.hosted.meta.HostedType;
import com.oracle.svm.reflect.target.MethodMetadataDecoderImpl;

import jdk.vm.ci.meta.JavaConstant;
import sun.reflect.annotation.TypeAnnotation;

@AutomaticFeature
class MethodMetadataFeature implements Feature {
    @Override
    public void afterRegistration(AfterRegistrationAccess access) {
        ImageSingletons.add(MethodMetadataEncoderFactory.class, new MethodMetadataEncoderImpl.Factory());
        ImageSingletons.add(MethodMetadataDecoder.class, new MethodMetadataDecoderImpl());
    }
}

public class ReflectionMetadata {

    static class FieldMetadata {
        final HostedType declaringType;
        final String name;
        final HostedType type;
        final int modifiers;
        final boolean trustedFinal;
        final String signature;
        final Annotation[] annotations;
        final TypeAnnotation[] typeAnnotations;

        FieldMetadata(HostedType declaringType, String name, HostedType type, int modifiers, boolean trustedFinal, String signature, Annotation[] annotations, TypeAnnotation[] typeAnnotations) {
            this.declaringType = declaringType;
            this.name = name;
            this.type = type;
            this.modifiers = modifiers;
            this.trustedFinal = trustedFinal;
            this.signature = signature;
            this.annotations = annotations;
            this.typeAnnotations = typeAnnotations;
        }
    }

    public static class MethodMetadata {
        final boolean complete;
        final HostedType declaringType;
        final String name;
        final HostedType[] parameterTypes;
        final int modifiers;
        final HostedType returnType;
        final HostedType[] exceptionTypes;
        final String signature;
        final Annotation[] annotations;
        final Annotation[][] parameterAnnotations;
        final TypeAnnotation[] typeAnnotations;
        final boolean hasRealParameterData;
        final MethodMetadataDecoderImpl.ReflectParameterDescriptor[] reflectParameters;
        final JavaConstant accessor;

        MethodMetadata(HostedType declaringClass, String name, HostedType[] parameterTypes, int modifiers, HostedType returnType, HostedType[] exceptionTypes, String signature,
                       Annotation[] annotations, Annotation[][] parameterAnnotations, TypeAnnotation[] typeAnnotations, boolean hasRealParameterData,
                       MethodMetadataDecoderImpl.ReflectParameterDescriptor[] reflectParameters, JavaConstant accessor) {
            this.complete = true;
            this.declaringType = declaringClass;
            this.name = name;
            this.parameterTypes = parameterTypes;
            this.modifiers = modifiers;
            this.returnType = returnType;
            this.exceptionTypes = exceptionTypes;
            this.signature = signature;
            this.annotations = annotations;
            this.parameterAnnotations = parameterAnnotations;
            this.typeAnnotations = typeAnnotations;
            this.hasRealParameterData = hasRealParameterData;
            this.reflectParameters = reflectParameters;
            this.accessor = accessor;
        }

        MethodMetadata(HostedType declaringClass, String name, HostedType[] parameterTypes) {
            this.complete = false;
            this.declaringType = declaringClass;
            this.name = name;
            this.parameterTypes = parameterTypes;
            this.modifiers = 0;
            this.returnType = null;
            this.exceptionTypes = null;
            this.signature = null;
            this.annotations = null;
            this.parameterAnnotations = null;
            this.typeAnnotations = null;
            this.hasRealParameterData = false;
            this.reflectParameters = null;
            this.accessor = null;
        }
    }

    public static class ConstructorMetadata {
        final boolean complete;
        final HostedType declaringType;
        final HostedType[] parameterTypes;
        final int modifiers;
        final HostedType[] exceptionTypes;
        final String signature;
        final Annotation[] annotations;
        final Annotation[][] parameterAnnotations;
        final TypeAnnotation[] typeAnnotations;
        final boolean hasRealParameterData;
        final MethodMetadataDecoderImpl.ReflectParameterDescriptor[] reflectParameters;
        final JavaConstant accessor;

        ConstructorMetadata(HostedType declaringClass, HostedType[] parameterTypes, int modifiers, HostedType[] exceptionTypes, String signature,
                       Annotation[] annotations, Annotation[][] parameterAnnotations, TypeAnnotation[] typeAnnotations, boolean hasRealParameterData,
                       MethodMetadataDecoderImpl.ReflectParameterDescriptor[] reflectParameters, JavaConstant accessor) {
            this.complete = true;
            this.declaringType = declaringClass;
            this.parameterTypes = parameterTypes;
            this.modifiers = modifiers;
            this.exceptionTypes = exceptionTypes;
            this.signature = signature;
            this.annotations = annotations;
            this.parameterAnnotations = parameterAnnotations;
            this.typeAnnotations = typeAnnotations;
            this.hasRealParameterData = hasRealParameterData;
            this.reflectParameters = reflectParameters;
            this.accessor = accessor;
        }

        ConstructorMetadata(HostedType declaringClass, HostedType[] parameterTypes) {
            this.complete = false;
            this.declaringType = declaringClass;
            this.parameterTypes = parameterTypes;
            this.modifiers = 0;
            this.exceptionTypes = null;
            this.signature = null;
            this.annotations = null;
            this.parameterAnnotations = null;
            this.typeAnnotations = null;
            this.hasRealParameterData = false;
            this.reflectParameters = null;
            this.accessor = null;
        }
    }
}
