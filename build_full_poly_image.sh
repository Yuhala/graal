#!/bin/bash
#
# Copyright (c) 2022 Peterson Yuhala, IIUN
#


#
# You must be in the graal directory as root here: ie make sure PWD = graal dir
# TODO: add script arguments to run these benchmarks automatically
#

APP_NAME="polyt_app"

APP_PKG="polytaint"
PKG_PATH="polytaint"

# The base directory is the "graal" folder
# At this point it should be the current working directory
BASE=$PWD


SVM_DIR="$BASE/substratevm"
SGX_DIR="$BASE/sgx"

## Truffle language components
SECUREL_COMPONENT="$BASE/components/secL-component.jar"
LLVM_COMPONENT="$BASE/components/llvm-toolchain-installable-java11-linux-aarch64-22.1.0.jar"
TRUFFLE_RUBY_COMPONENT="$BASE/components/llvm-toolchain-installable-java11-linux-amd64-22.1.0.jar"
GRAAL_PYTHON="$BASE/components/python-installable-svm-java11-linux-amd64-22.1.0.jar"
ANTLR4="$BASE/components/antlr-repackaged-4.0.jar"
ANTLR4_COMPLETE="$BASE/components/antlr-4.10.1-complete.jar"
ANTLR4_RT="$BASE/components/antlr4-runtime-4.8.jar"


APP_DIR="$SVM_DIR/$APP_NAME"
#JAVA="$JAVA_HOME/bin/java"

LIB_BASE="$APP_DIR/lib/*"

SECL_LIBS="$SVM_DIR/secl_libs/antlr4-runtime-4.8.jar"

## ------------------- GraalVM variables -------------------
graal_build=($BASE/vm/latest_graalvm/*)
## The graalvm home is the only folder in latest_graalvm
graalvm_dev=${graal_build[0]}
graalvm_home=$graalvm_dev
GRAAL_SDK=""
native_image="$graalvm_home/bin/native-image"
VM_DIR="$BASE/vm"

## ---------------------------------------------------------



#mx native-image -cp $CP $NATIVE_IMG_OPTS $APP_PKG.$MAIN

## ----------------- Building GraalVM "vm" suite -------------
#
# Build the vm suite.
# This also builds the svm suite
#
function build_graalvm {
    #TODO: set jvmci as java home
    echo "+++++++++++++++++++++ Building the VM suite ++++++++++++++++"
    cd $VM_DIR
    #mx --disable-polyglot --disable-libpolyglot --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath graalvm-show
    export MX_PYTHON="python3"
    mx --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath build
    
}

## ---------------- Install language components into Graal dev build ---------
function install_language_components() {
    $gu install -L $SECUREL_COMPONENT
}
## ----------------------------------------------------------


#build_graalvm
#exit 1

## ----------------------------------------------------------


## ----------------- Reset GraalVM variables ------------------
echo "+++++++++++++++++++++ Setting graalvm variables ++++++++++++++++"
graal_build=($BASE/vm/latest_graalvm/*)
## The graalvm home is the only folder in latest_graalvm
graalvm_dev=${graal_build[0]}
graalvm_home=$graalvm_dev
native_image="$graalvm_home/bin/native-image"
gu="$graalvm_home/bin/gu"
GRAAL_SDK=""
echo "+++++++++++++++++++++ graalvm_home: $graalvm_home++++++++++++++++"
## ------------------------------------------------------------


## ---------------- Install components --------------
#install_language_components

## --------------------------------------------------


function build_svm {
    echo "-------------building svm-------------------"
    mx clean
    rm -rf svmbuild
    mx build
    echo "-------------svm build complete-------------"
}





## ---------------- other variables ------------------
# NB: this does not set the "real" env variable
JAVA_HOME=$graalvm_dev
JAVAC="$JAVA_HOME/bin/javac"

echo "JAVA HOME IS: $JAVA_HOME"

## ----------------------------------------------------


## ----------- Application related variables --------
CP=$LIB_BASE:$graalvm_dev:$GRAAL_SDK:$SVM_DIR:$APP_DIR
polyt_trusted="Trusted"
polyt_untrusted="Untrusted"

DB="$APP_DIR/data"



#clean old objects and rebuild svm if changed
OLD_OBJS=(/tmp/main.o main.so $APP_DIR/*.class $SGX_DIR/Enclave/graalsgx/polytaint/*.o $SGX_DIR/App/graalsgx/polytaint/*.o)
cd $SVM_DIR
echo "------------- Removing old objects -----------"
for obj in $OLD_OBJS; do
    rm $obj
done




#clean app classes
echo "--------------- Cleaning $APP_NAME classes -------------"
find $APP_DIR -name "*.class" -type f -delete



#compile app classes
BUILD_OPTS="-Xlint:unchecked -Xlint:deprecation"


function build_trusted_app {
    echo "------------ Compiling Trusted Application -------------"
    $JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$polyt_trusted.java
}


build_trusted_app


#echo "------------ Running $APP_NAME on normal JVM  ----------"
#$JAVA_HOME/bin/java  -cp $CP $APP_PKG.$MAIN





function run_app_with_tracer {
    echo "------------ Running trusted component on JVM with tracing agent ----------"
    rm -rf "./META-INF/native-image"
    mkdir -p META-INF/native-image
    $JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$polyt_trusted
}


#run application in jvm to generate any useful configuration files: reflection, serialization, dynamic class loading etc
#echo "--------------- Running $APP_PKG on JVM to generate useful config files-----------"
#mkdir -p META-INF/native-image
#$JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$MAIN 100




## ----------- Native image build options --------

INIT_AT_RUN_TIME="--initialize-at-run-time=com.oracle.truffle.secureL.parser.SecureLLexer,com.oracle.truffle.secureL.parser.SecureLParser"

#NATIVE_IMG_OPTS="--shared --sgx --no-fallback --language:python --language:secl --allow-incomplete-classpath -O0 -H:+ReportExceptionStackTraces
#--trace-class-initialization=org.antlr.v4.runtime.atn.Transition"

#-R:MinHeapSize=2g -R:MaxHeapSize=2g --gc=epsilon

NATIVE_IMG_OPTS="--shared --sgx --no-fallback -R:MaxHeapSize=8g --gc=epsilon --language:js --language:secl --allow-incomplete-classpath -O0 -H:+ReportExceptionStackTraces
--trace-class-initialization=org.antlr.v4.runtime.atn.Transition"

NORMAL_OPTS="--no-fallback --language:js --allow-incomplete-classpath -O0 -H:+ReportExceptionStackTraces
--trace-class-initialization=org.antlr.v4.runtime.atn.Transition"

#NATIVE_IMG_OPTS="--shared --sgx --no-fallback --language:python --allow-incomplete-classpath -O0"

#NATIVE_IMG_OPTS="--no-fallback --language:js --language:python --allow-incomplete-classpath -O0"


LOCAL_OPT=-H:+LocalizationOptimizedMode 
#-H:+TraceClassInitialization
#--allow-incomplete-classpath
#--trace-class-initialization=org.springframework.util.ClassUtils

# Trusted and untrusted reflection configuration files
REFLECT_CONFIG_IN=$APP_DIR/$PKG_PATH/reflect-config-in.json


REFLECT_OPT_IN="-H:ReflectionConfigurationFiles=$REFLECT_CONFIG_IN"

#
# clean any files from partitioning module
# 
function clean_polytaint_files() {
    rm $SGX_DIR/Enclave/graalsgx/polytaint/*.h
    rm $SGX_DIR/Enclave/graalsgx/polytaint/*.cpp
    rm $SGX_DIR/App/graalsgx/polytaint/*.h
    rm $SGX_DIR/App/graalsgx/polytaint/*.cpp    
}

function cleanOldGen {
    rm $SVM_DIR/$PKG_PATH/$polyt_trusted.java
    rm $SVM_DIR/$PKG_PATH/$polyt_untrusted.java
    rm $SVM_DIR/$PKG_PATH/$polyt_trusted.class
    rm $SVM_DIR/$PKG_PATH/$polyt_untrusted.class
}

clean_polytaint_files


function build_full_sgx_image {
    echo "--------------- Building Trusted SGX native image -----------"
    $native_image -cp $CP $NATIVE_IMG_OPTS $INIT_AT_RUN_TIME $LOCAL_OPT $APP_PKG.$polyt_trusted
    
    echo "--------------- Copying generated files to trusted module -----------"
    #copy new created object file to sgx module
    cp /tmp/main.o $SGX_DIR/Enclave/graalsgx/
    cp /tmp/main.o $SGX_DIR/App/graalsgx/
    #copy generated headers to sgx module; graal entry points are defined here
    mv $SVM_DIR/*.h $SGX_DIR/Include/
}


function build_normal_image {
    echo "--------------- Building normal native image -----------"
    $native_image -cp $CP $NORMAL_OPTS $INIT_AT_RUN_TIME $LOCAL_OPT $APP_PKG.$polyt_trusted
    
    
}


run_app_with_tracer

build_full_sgx_image

#build_normal_image









