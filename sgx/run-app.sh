#!/bin/bash


#for ((i=$min; i<=$max; i+=$step))
#do
#    ./app $i
#done

DATA="/home/petman/projects/graal-tee/substratevm/graphchi/data"
#clean
rm -rf $DATA/gen.txt.* $DATA/*.bin

echo "----------------- Running sgx app ----------------"
./app 6

#remove shards
#rm -rf $DATA/data.* $DATA/*.bin
#rm -rf $DATA/soc1m.* $DATA/*.bin
rm -rf $DATA/gen.txt.* $DATA/*.bin