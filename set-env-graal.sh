#!/bin/bash
# 
# Author: Peterson Yuhala
# adds mx to path and points java home to jdk-jvmci
#

#export PATH=$PWD/vm/latest_graalvm/graalvm-a7a83e7967-java11-22.1.0-dev/bin:$PATH
#export JAVA_HOME=$PWD/vm/latest_graalvm/graalvm-a7a83e7967-java11-22.1.0-dev

#
# Each graal build has a different name so we need a more practical
# way of adding the new directory to the path
#

for d in $PWD/vm/latest_graalvm
    do
        export PATH="$d/bin:$PATH"
        export JAVA_HOME="$d"
    done