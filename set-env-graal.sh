#!/bin/bash
# 
# Author: Peterson Yuhala
# This script updates adds graalvm dev build to the path 
# and modifies JAVA_HOME to point to it.
#

#GRAALVM_DEV="graalvm-d6856a2436-java11-22.1.0-dev"
DIR_LIST=($PWD/vm/latest_graalvm/*)

GRAALVM_DEV=${DIR_LIST[0]}

echo "+++++++++++++++++++++ GraalVM-dev home is: $GRAALVM_DEV" 

export PATH=$GRAALVM_DEV/bin:$PATH
export JAVA_HOME=$GRAALVM_DEV

