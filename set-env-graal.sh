#!/bin/bash
# 
# Author: Peterson Yuhala
# adds mx to path and points java home to jdk-jvmci
#

GRAALVM_DEV="graalvm-d6856a2436-java11-22.1.0-dev"

export PATH=$PWD/vm/latest_graalvm/$GRAALVM_DEV/bin:$PATH
export JAVA_HOME=$PWD/vm/latest_graalvm/$GRAALVM_DEV



#
# Each graal build has a different name so we need a more practical
# way of adding the new directory to the path
#

# TODO