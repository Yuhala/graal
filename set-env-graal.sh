#!/bin/bash
# 
# Author: Peterson Yuhala
# adds mx to path and points java home to jdk-jvmci
#

export PATH=$PWD/vm/latest_graalvm/graalvm-26855d62ff-java8-21.1.0-dev/bin:$PATH
export JAVA_HOME=$PWD/vm/latest_graalvm/graalvm-26855d62ff-java8-21.1.0-dev
