#!/bin/bash

#
# This script downloads and installs some 
# ubuntu 18.04 packages needed for the SGX debug tool: sgx-gdb
#

sudo apt install wget

# update source list
echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' | sudo tee /etc/apt/sources.list.d/intel-sgx.list
wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | sudo apt-key add -
sudo apt update

sudo apt install libsgx-enclave-common-dbgsym libsgx-urts-dbgsym