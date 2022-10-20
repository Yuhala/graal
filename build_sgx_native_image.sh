#!/bin/bash
#
# Copyright (c) 2022 Peterson Yuhala, IIUN
#


#
# You must be in the graal directory as root here: ie make sure PWD = graal dir
# TODO: add script arguments to run these benchmarks automatically
#

APP_NAME="smartc"

APP_PKG="iiun.smartc"
PKG_PATH="iiun/smartc"

# The base directory is the "graal" folder
# At this point it should be the current working directory
BASE=$PWD


SVM_DIR="$BASE/substratevm"
SGX_DIR="$BASE/sgx"

## Truffle language components
SECUREL_COMPONENT="$BASE/components/secL-component.jar"
LLVM_COMPONENT="$BASE/components/llvm-toolchain-installable-java8-linux-amd64-21.1.0.jar"
TRUFFLE_RUBY_COMPONENT="$BASE/components/llvm-toolchain-installable-java8-linux-amd64-21.1.0.jar"



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
    mx --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath build
    
}

## ---------------- Install language components into Graal dev build ---------
function install_language_components() {
    $gu install -L $SECUREL_COMPONENT
}
## ---------------------------------------------------------------------------

build_graalvm

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
# install_language_components

## --------------------------------------------------


## ---------------- other variables ------------------
JAVA_HOME=$graalvm_dev
JAVAC="$JAVA_HOME/bin/javac"

echo "JAVA HOME IS: $JAVA_HOME"

## -------------------------------------------------------


## ----------- Application related variables --------
CP=$LIB_BASE:$graalvm_dev:$GRAAL_SDK:$SVM_DIR:$SECL_LIBS:$APP_DIR
MAIN="Main"

DB="$APP_DIR/data"


#clean old objects and rebuild svm if changed
OLD_OBJS=(/tmp/main.o main.so $APP_DIR/*.class $SGX_DIR/Enclave/graalsgx/*.o $SGX_DIR/App/graalsgx/*.o)
cd $SVM_DIR

echo "------------- Removing old objects -----------"
for obj in $OLD_OBJS; do
    rm $obj
done


function build_svm {
    mx clean
    rm -rf svmbuild
    mx build
}

#build_svm

#exit 1
#clean app classes
echo "--------------- Cleaning $APP_NAME classes -----------"
find $APP_DIR -name "*.class" -type f -delete

#compile app classes
BUILD_OPTS="-Xlint:unchecked -Xlint:deprecation"


function build_java_app {
    echo "------------ Compiling $APP_NAME application -----------"
    $JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$MAIN.java 
}

function run_java_app_with_tracer {
    echo "------------ Running $APP_NAME on normal JVM with tracing agent ----------"
    rm -rf META-INF/native-image
    mkdir -p META-INF/native-image
    $JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$MAIN
}

build_java_app

#run_java_app_with_tracer

echo "--------------- Building $APP_NAME SGX native image -----------"
NATIVE_IMG_OPTS="--shared --sgx --no-fallback --language:js --allow-incomplete-classpath"

#NATIVE_IMG_OPTS="--no-fallback --language:js --allow-incomplete-classpath -O0"


#-H:+TraceClassInitialization


#mx native-image -cp $CP $NATIVE_IMG_OPTS $APP_PKG.$MAIN
HEAP_OPTS="-R:MinHeapSize=1g -R:MaxHeapSize=1g"
LOCAL_OPT=-H:+LocalizationOptimizedMode 
REFLECT_CONFIG="$APP_DIR/$PKG_PATH/reflect-config-in.json"
REFLECT_OPT="-H:ReflectionConfigurationFiles=$REFLECT_CONFIG"

$native_image -cp $CP $NATIVE_IMG_OPTS $LOCAL_OPT $REFLECT_OPT $APP_PKG.$MAIN


#mx native-image -cp $CP $NATIVE_IMG_OPTS $APP_PKG.$MAIN

function clean_polytaint_files() {
    rm $SGX_DIR/Enclave/graalsgx/polytaint/*.h
    rm $SGX_DIR/Enclave/graalsgx/polytaint/*.cpp
    rm $SGX_DIR/App/graalsgx/polytaint/*.h
    rm $SGX_DIR/App/graalsgx/polytaint/*.cpp
}

clean_polytaint_files

#copy new created object file to sgx module
cp /tmp/main.o $SGX_DIR/Enclave/graalsgx/
cp /tmp/main.o $SGX_DIR/App/graalsgx/
#copy generated headers to sgx module; graal entry points are defined here
mv $SVM_DIR/*.h $SGX_DIR/Include/
