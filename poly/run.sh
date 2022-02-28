
#!/bin/bash
#
# Copyright (c) 2020 Peterson Yuhala, IIUN
# Test
#


#
# You must be in the graal-sgx directory as root here: ie make sure PWD = graal-sgx dir
# TODO: add script arguments to run these benchmarks automatically
#


JAVAC="$JAVA_HOME/bin/javac"
JAVA="$JAVA_HOME/bin/java"

LIB_BASE="$APP_DIR/lib/*"

GRAAL_HOME="$PWD"
GRAAL_SDK="$PWD/sdk/mxbuild/dists/graal-sdk.jar"
POLYGLOT_API="$PWD/sdk/latest_graalvm_home/lib/polyglot/polyglot-native-api.jar"
TRUFFLE_API="$PWD/sdk/mxbuild/dists/truffle-api.jar"

JARS="$PWD/jars/*"

#simple language ("secure language") component
SL="$SVM_DIR/components/sl-component.jar"

CP=$LIB_BASE:$JARS:$GRAAL_HOME:$GRAAL_SDK:$TRUFFLE_API:$POLYGLOT_API:$APP_DIR:$SVM_DIR



echo "--------------- Compiling application -----------"
#$JAVAC -cp $CP $BUILD_OPTS $APP_DIR/$PKG_PATH/$MAIN.java $OTHERS



echo "--------------- Running polyglot app ------------"
$JAVAC -cp /home/ubuntu/sgx-truffle/graal/secureL/launcher-22.0.0.2.jar:. -Dtruffle.class.path.append=/homentu/sgx-truffle/graal/secureL/simplelanguage.jar HelloPolyglot


