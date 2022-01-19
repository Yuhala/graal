#!/bin/bash

#NB: we can optimize all these scripts further to run 
#the specific benchmarks via options etc

#change directory to present dir
cd "$(dirname "$0")"

#step=10000

#for ((i=$min; i<=$max; i+=$step))
#do
#    ./app $i
#done

#for paldb
#python3 ./bench.py

#build sgx app
make clean;make

perc=$1

python3 $PWD/sim.py $perc