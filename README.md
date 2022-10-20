# Graal SGX Project
- `Project progress`: ![70%](https://progress-bar.dev/70)
- This branch represents a modification of graal vm CE to run full native images in Intel SGX enclaves.


## Project goals/milestones
- The [project's](docs/ero-proposal.pdf) main goal is to extend GraalVM native images with TEE (Intel SGX) functionality. We have the following milestones: 
- [x] Running a full native image java program inside the enclave for a start (this branch; the rest on `jtransform` branch)
- [x] Produce 2 separate binaries with secure/unsecure methods.
- [x] Generation of transition routines for ecalls/ocalls.
- [x] Creating proxy and mirror objects across both runtimes (primitive params only).
- [x] GC modifications to synchronize proxy/mirror object destructions (proxy cleaner class).
- [ ] Object parameter and return types via serialization.
- [ ] Validating our approach with a motivating example e.g executing smart contracts.
- [ ] Adding PM functionality to our system. 



## Substrate VM
- Substrate VM is the component which creates native images and is the core repo we are working on.
- SVM supports AoT compilation of Java apps into standalone executable images (i.e native images).
- Native image includes the necessary components like mem management and thread scheduling from substrate VM: deoptimizer, GC, thread scheduling etc

## Building and running a substrate VM native image in an Intel SGX enclave on Linux.  
- Clone this repo to a directory in local environment, which we will call `graal-sgx-root`. Unless stated otherwise, all `cd` commands assume `graal-sgx-root` as the top working directory.
```
mkdir graal-sgx-root && cd graal-sgx-root
git clone git@gitlab.com:Yuhala/graal-sgx.git

```

### SGX Installation
- We created a script to install SGX for Ubuntu based systems: `16.04`, `18.04`, and `20.04`. It was tested on both `18.04` and `20.04`
- Copy the `sgx-install.sh` script from `graal-tee` folder into `graal-sgx-root`: 
```
cp graal-tee/sgx-install.sh .

```
- The script by default will install SGX tools with debug information included. To install the tools without debug information, change the value of `debug_info` to `0`.
- Run the SGX install script:
```
./sgx-install.sh

```
- If your hardware does not support SGX, the necessary packages will still be installed; however you will have to use SGX in simulation mode in your makefiles (`SGX_MODE=SIM`).
- To use the SGX SDK custom debugger `sgx-gdb`, install these packages: `sudo apt libsgx-enclave-common-dbgsym libsgx-urts-dbgsym`.

### Installing graalvm tools
- Note: for `Ubuntu 20.04`,  you may need to properly configure an `http_proxy` to build graal properly. Our tests with `Ubuntu 18.04` worked without any issues.
- Install the [mx](https://github.com/graalvm/mx) build tool:

```
git clone https://github.com/graalvm/mx.git

```
- GraalVM's JIT compiler works with the default JVM as a plugin with the help of the JVM compiler interface (JVMCI), and thus requires a JDK which supports a graal-compatible version of JVMCI. You can find a compatible version in the `graal-tee` folder: `openjdk-8u282+08-jvmci-21.1-b01-linux-amd64.tar.gz`. Other compatible versions can be found here: [jvmci releases](https://github.com/graalvm/graal-jvmci-8/releases).

- Copy and extract this file in `graal-sgx-root`.

```
cp graal-tee/openjdk-8u282+08-jvmci-21.1-b01-linux-amd64.tar.gz . 
tar -xvf openjdk-8u282+08-jvmci-21.1-b01-linux-amd64.tar.gz
openjdk1.8.0_282-jvmci-21.1-b01

```
- Copy the native image agent shared library to the `jre/lib/amd64` folder of this jdk. The native image agent generates useful config files for native images at runtime.

```
cp graal-tee/libnative-image-agent.so openjdk1.8.0_282-jvmci-21.1-b01/jre/lib/amd64

```

- Add `mx` to path and point `JAVA-HOME` to the jdk with jvmci:
```
cp graal-tee/config-env .
source config-env

```



### Build and run a native image in an Intel SGX enclave
- Here we will build a simple native image java program with SVM and run it in an Intel SGX enclave. 
- The program simulates a smart contract for transferring assets between peers.
- The application package is `substratevm/smartc` directory. SVM AoT compiles this program into an object file: `main.o` which will be linked to enclave libraries in the `sgx module` to build an Intel SGX enclave application.
-  CD into `graal-tee` directory and run the script: `build.sh` to build and copy `main.o` to the `sgx module`:
```
cd graal-tee
./build.sh 

```
- Note the path to the static library: `liblibchelper.a` at the end of the build process; it would be helpful in case the present version in `sgx/Common` is not compatible with your linker.
- The path to the other static libs should probably be: `sdk/mxbuild/linux-amd64/GRAALVM_change-this-path_JAVA11/graalvm-dev/lib/static/linux-amd64/glibc`
- The `cpuid.c` file was removed from the graal libchelper due to multiple definitions already found in the sgx-module. 

- CD into the `sgx module` and run the script `make clean && make` to build the final enclave application. 

```bash
cd graal-tee/sgx
make clean && make

```
- Run the resulting program `app`.

```bash
./app

```

## Possible build errors
- In the event of linker errors with `liblibchelper.a`, remove the latter from `sgx/Common` folder and replace it with the one built during the build process above which can be found here: `substratevm/mxbuild/linux-amd64/SVM_HOSTED_NATIVE/linux-amd64/liblibchelper.a`

- Rebuild the SGX program.

## Other resources
- Article on Graal compiler by Chris Seaton: https://chrisseaton.com/truffleruby/jokerconf17/
- Medium article on native images by Christian Wimmer: https://medium.com/graalvm/isolates-and-compressed-references-more-flexible-and-efficient-memory-management-for-graalvm-a044cc50b67e
