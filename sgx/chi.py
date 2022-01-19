#!/usr/bin/env python

#
# Author: Peterson Yuhala
# Graal-SGX Benchmarking script for graphchi
#

import os
import subprocess
import time
import csv
import math
import statistics
from random import randint

# number of warm up runs
WARMUP = 1
# number of runs
NUM_RUNS = 5


PROCNAME = "app"
BINPATH = "./"
TEMP = BINPATH + "results/temp.csv"
MAIN_RESULTS = BINPATH + "results/main.csv"

# TODO:get graal-tee base
# rm -rf $DATA/gen.txt.* $DATA/*.bin
DATA_BASE = "/home/petman/projects/graal-tee/substratevm/graphchi/data"


# results for each NUM_RUNS runs
run_results = []

# read values for the runs in temp file
# file read as list of lists
# #shards,engineTime,SharderTime,Totaltime


def get_run_results():
    res = []
    with open(TEMP, 'r') as file:
        reader = csv.reader(file, delimiter=",")
        res = list(reader)

    return res


# calculate mean/median value from list of list
#  #shards,engineTime,SharderTime,Totaltime
def getMeans(results):
    engineTimes = []
    shardTimes = []
    totalTimes = []
    for l in results:
        engineTimes.append(float(l[1]))
        shardTimes.append(float(l[2]))
        totalTimes.append(float(l[3]))

    resultMeans = []
    resultMeans.append(statistics.mean(engineTimes))
    resultMeans.append(statistics.mean(shardTimes))
    resultMeans.append(statistics.mean(totalTimes))

    return resultMeans


# write results to main.csv
def write_results(results, num_shards):

    with open(MAIN_RESULTS, "a", newline='') as res_file:
        writer = csv.writer(res_file, delimiter=',')
        writer.writerow([str(num_shards)]+getMeans(results))

    results.clear()




def clean(filename):
    if os.path.exists(filename):
        os.remove(filename)
    else:
        print(f'{filename} does not exist..')


def cleanShards():
    # rm -rf $DATA/gen.txt.* $DATA/*.bin
    shards = DATA_BASE+"/gen.txt.*"
    binData = DATA_BASE+"/*.bin"
    command = "rm -rf "+shards+" "+binData
    os.system(command)


def run_bench():
    # remove previous bench results
    clean(MAIN_RESULTS)
    # remove temp file
    clean(TEMP)
    cleanShards()

    numShards = 1
    maxShards = 6

    # do warm up
    for i in range(WARMUP):
        proc = subprocess.Popen([BINPATH + PROCNAME, str(numShards)])
        print(
            f'----------------Running graphchi warmup. Num shards: {numShards} -----------------')
        proc.wait()
        clean(TEMP)
        cleanShards()

    while(numShards <= maxShards):
        for i in range(NUM_RUNS):
            proc = subprocess.Popen([BINPATH + PROCNAME, str(numShards)])
            print(
                f'----------------Running graphchi bench. Num shards: {numShards} -----------------')
            proc.wait()
            cleanShards()

        # read run results
        results = get_run_results()
        print(f'Temp results: {results}')
        write_results(results, numShards)
        clean(TEMP)

        numShards += 1


run_bench()
