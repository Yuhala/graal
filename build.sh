
#!/bin/bash
#
# Copyright (c) 2020 Peterson Yuhala, IIUN
# Test
#


#
# You must be in the graal-sgx directory as root here: ie make sure PWD = graal-sgx dir
# TODO: add script arguments to run these benchmarks automatically
#

APP_NAME="smartc"

APP_PKG="iiun.smartc"
PKG_PATH="iiun/smartc"

SVM_DIR="$PWD/substratevm"
SGX_DIR="$PWD/sgx"

APP_DIR="$SVM_DIR/$APP_NAME"
JAVAC="$JAVA_HOME/bin/javac"
#JAVA="$JAVA_HOME/bin/java"

LIB_BASE="$APP_DIR/lib/*"

GRAAL_HOME="$PWD/sdk/latest_graalvm_home/jre/lib/boot/graal-sdk.jar"
GRAAL_HOME="$PWD/sdk/latest_graalvm_home/**/*.jar"
POLYGLOT_LIBS="$PWD/sdk/latest_graalvm_home/lib/polyglot/polyglot-native-api.jar"
TRUFFLE_LIBS="$PWD/sdk/latest_graalvm_home/lib/truffle/truffle-api.jar"

CP=$LIB_BASE:$GRAAL_HOME:$APP_DIR:$SVM_DIR:$TRUFFLE_LIBS

MAIN="Main"

OTHERS=""
DB="$APP_DIR/data"


# native image build options
SVM_OPTS=
#"-H:+UseOnlyWritableBootImageHeap -H:+ReportExceptionStackTraces"

#clean old objects and rebuild svm if changed
OLD_OBJS=(/tmp/main.o main.so $APP_DIR/*.class $SGX_DIR/Enclave/graalsgx/*.o $SGX_DIR/App/graalsgx/*.o)
cd $SVM_DIR
echo "---------------Removing old objects -----------"
for obj in $OLD_OBJS; do
    rm $obj
done

# NB: always rebuild svm after changing branches
# Especially after switching btw branches full and partitioned

function build_svm {
    mx clean
    rm -rf svmbuild
    mx build
}

if [[ $1 -eq 1 ]]
    then 
        build_svm
fi

#exit 1
#clean app classes
echo "--------------- Cleaning $APP_NAME classes -----------"
find $APP_DIR -name "*.class" -type f -delete

#compile app classes
BUILD_OPTS="-Xlint:unchecked -Xlint:deprecation"

echo "--------------- Compiling $APP_NAME application -----------"
$JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$MAIN.java $OTHERS

#run application in jvm to generate any useful configuration files: reflection, serialization, dynamic class loading etc
#echo "--------------- Running $APP_PKG on JVM to generate useful config files-----------"
#mkdir -p META-INF/native-image
#$JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=META-INF/native-image -cp $CP $APP_PKG.$MAIN 

$JAVA_HOME/bin/java -cp $CP $APP_PKG.$MAIN 

#$JAVA_HOME/bin/native-image $APP_PKG.Main

#$JAVA_HOME/bin/jar cvf $APP_DIR/smartc.jar $APP_DIR/*
#chmod 764 $APP_DIR/smartc.jar

#exit 1
# compile and build new native image
# echo $APP_DIR
#$JAVAC -sourcepath . -cp $GRAAL_HOME:$SGX_ANNOTATIONS/graal-sgx.jar:$APP_DIR/smartc.jar Main.java


echo "--------------- Building $APP_NAME native image -----------"
NATIVE_IMG_OPTS="--shared --sgx --no-fallback --allow-incomplete-classpath"

#NATIVE_IMG_OPTS="--shared --no-fallback --allow-incomplete-classpath"

#-H:+TraceClassInitialization 
#--allow-incomplete-classpath
#--trace-class-initialization=org.springframework.util.ClassUtils

mx native-image -cp $CP $NATIVE_IMG_OPTS $APP_PKG.$MAIN

#copy new created object file to sgx module
cp /tmp/main.o $SGX_DIR/Enclave/graalsgx/
cp /tmp/main.o $SGX_DIR/App/graalsgx/
#copy generated headers to sgx module; graal entry points are defined here
mv $SVM_DIR/*.h $SGX_DIR/Include/
