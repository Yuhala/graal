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
    export MX_PYTHON="python3"
    mx --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath build
    
}


## ---------------- Install language components into Graal dev build ---------
function install_language_components() {
    $gu install -L $SECUREL_COMPONENT
}
## ---------------------------------------------------------------------------

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




## ---------------- other variables ------------------
# NB: this does not set the "real" env variable
JAVA_HOME=$graalvm_dev
JAVAC="$JAVA_HOME/bin/javac"

echo "JAVA HOME IS: $JAVA_HOME"

## -------------------------------------------------------


## ----------- Application related variables --------
CP=$LIB_BASE:$graalvm_dev:$GRAAL_SDK:$SVM_DIR:$SECL_LIBS:$APP_DIR
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


function build_svm {
    mx clean
    rm -rf svmbuild
    mx build
}
#build_svm



#clean app classes
echo "--------------- Cleaning $APP_NAME classes -----------"
find $APP_DIR -name "*.class" -type f -delete



#compile app classes
BUILD_OPTS="-Xlint:unchecked -Xlint:deprecation"



function build_trusted_app {
    echo "------------ Compiling Trusted Application -----------"
    $JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$polyt_trusted.java
}


function build_untrusted_app {
    echo "------------ Compiling Untrusted Application -----------"
    $JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$polyt_untrusted.java
}


build_trusted_app
build_untrusted_app


#echo "------------ Running $APP_NAME on normal JVM  ----------"
#$JAVA_HOME/bin/java  -cp $CP $APP_PKG.$MAIN


function clean_polytaint_files {
    rm $APP_DIR/$PKG_PATH/*.java
    rm $APP_DIR/$PKG_PATH/*.class
}

function run_trusted_component_with_tracer {
    echo "------------ Running trusted component with on normal JVM with tracing agent ----------"
    #rm -rf "./META-INF/native-image"
    #mkdir -p META-INF/native-image
    $JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$polyt_trusted
    #add reflection configuration
    cat $APP_DIR/$PKG_PATH/reflect-config-in.json > $APP_DIR/$PKG_PATH/config-in/reflect-config.json
    cp $APP_DIR/$PKG_PATH/config-in/reflect-config.json META-INF/native-image
}


function run_untrusted_component_with_tracer {
    echo "------------ Running trusted component with on normal JVM with tracing agent ----------"
    #rm -rf "./META-INF/native-image"
    #mkdir -p META-INF/native-image
    $JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$polyt_untrusted
    #add reflection configuration
    cat $APP_DIR/$PKG_PATH/reflect-config-out.json > $APP_DIR/$PKG_PATH/config-out/reflect-config.json
    cp $APP_DIR/$PKG_PATH/config-out/reflect-config.json META-INF/native-image
}




#run application in jvm to generate any useful configuration files: reflection, serialization, dynamic class loading etc
#echo "--------------- Running $APP_PKG on JVM to generate useful config files-----------"
#mkdir -p META-INF/native-image
#$JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$MAIN 100




## ----------- Native image build options --------
#NATIVE_IMG_OPTS="--shared --sgx --no-fallback --language:js --allow-incomplete-classpath -O0"
LOCAL_OPT=-H:+LocalizationOptimizedMode 

#-R:MinHeapSize=3g -R:MaxHeapSize=3g

INIT_AT_RUN_TIME="--initialize-at-run-time=com.oracle.truffle.secureL.parser.SecureLLexer,com.oracle.truffle.secureL.parser.SecureLParser"

NATIVE_IMG_OPTS="--shared --sgx --no-fallback -R:MaxHeapSize=8g --gc=epsilon  --language:js --language:secl --allow-incomplete-classpath -O0 -H:+ReportExceptionStackTraces
  --trace-class-initialization=org.antlr.v4.runtime.atn.Transition"

#-H:+TraceClassInitialization
#--allow-incomplete-classpath
#--trace-class-initialization=org.springframework.util.ClassUtils

# Trusted and untrusted reflection configuration files
REFLECT_CONFIG_IN=$APP_DIR/$PKG_PATH/reflect-config-in.json
REFLECT_CONFIG_OUT=$APP_DIR/$PKG_PATH/reflect-config-out.json

REFLECT_OPT_IN="-H:ReflectionConfigurationFiles=$REFLECT_CONFIG_IN"
REFLECT_OPT_OUT="-H:ReflectionConfigurationFiles=$REFLECT_CONFIG_OUT"


function build_trusted_image {
    echo "--------------- Building Trusted SGX native image -----------"
    $native_image --verbose -cp $CP $NATIVE_IMG_OPTS $INIT_AT_RUN_TIME $LOCAL_OPT $REFLECT_OPT_IN $APP_PKG.$polyt_trusted
    
    echo "--------------- Copying generated files to trusted module -----------"
    mv /tmp/main.o $SGX_DIR/Enclave/graalsgx/
    mv $SVM_DIR/*.h $SGX_DIR/Enclave/graalsgx/polytaint
}


function build_untrusted_image {
    echo "--------------- Building Untrusted SGX native image -----------"
    $native_image --verbose -cp $CP $NATIVE_IMG_OPTS $INIT_AT_RUN_TIME $LOCAL_OPT $REFLECT_OPT_OUT $APP_PKG.$polyt_untrusted
    
    echo "--------------- Copying generated files to untrusted module -----------"
    mv /tmp/main.o $SGX_DIR/App/graalsgx/
    mv $SVM_DIR/*.h $SGX_DIR/App/graalsgx/polytaint
}

#clean_polytaint_files

run_trusted_component_with_tracer
build_trusted_image

run_untrusted_component_with_tracer
build_untrusted_image






