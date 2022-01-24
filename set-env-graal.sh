#!/bin/bash
# 
# Author: Peterson Yuhala
# adds mx to path and points java home to jdk-jvmci
#

export PATH=$PWD/vm/latest_graalvm/graalvm-dev/bin:$PATH
export JAVA_HOME=$PWD/vm/latest_graalvm/graalvm-dev



#
# Each graal build has a different name so we need a more practical
# way of adding the new directory to the path
#

# TODO