#!/bin/bash

#make clean;make
rm enclave.signed.so
#./sgx_prot/sgx_prot_rw -i enclave.so -o enclave.so 
./sgx_prot/elf-permissions/elf-perm-change -i enclave.so -o enclave.so 
make prot 
