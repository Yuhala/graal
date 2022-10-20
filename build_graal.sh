#!/bin/bash
#
# Copyright (c) 2022 Peterson Yuhala, IIUN
#


#
# You must be in the graal directory as root here: ie make sure PWD = graal dir
# TODO: add script arguments to run these benchmarks automatically
#


# The base directory is the "graal" folder
# At this point it should be the current working directory
BASE=$PWD




## ------------------- GraalVM variables -------------------
graal_build=($BASE/vm/latest_graalvm/*)
## The graalvm home is the only folder in latest_graalvm
graalvm_dev=${graal_build[0]}
graalvm_home=$graalvm_dev
GRAAL_SDK=""
native_image="$graalvm_home/bin/native-image"
VM_DIR="$BASE/vm"

## ---------------------------------------------------------



## ----------------- Building GraalVM "vm" suite -------------
#
# Build the vm suite.
# This also builds the svm suite
#
function build_graalvm {
    #TODO: set jvmci as java home
    echo "+++++++++++++++++++++ Building the VM suite of GraalVM ++++++++++++++++"
    cd $VM_DIR
    #mx --disable-polyglot --disable-libpolyglot --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath graalvm-show
    export MX_PYTHON="python3"
    mx --env sgx  --extra-image-builder-argument=--allow-incomplete-classpath build
    
}


build_graalvm
exit 1