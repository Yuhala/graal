#!/usr/bin/env python

#
# Author: Peterson Yuhala
# Graal-SGX Benchmarking script
#

import os
import sys
import subprocess
import time
import csv
import math
import statistics
from random import randint

# number of warm up runs
WARMUP = 1
# number of runs
NUM_RUNS = 10

BENCH_NAME = sys.argv[1]
PROCNAME = "app"
BINPATH = "./"
TEMP = BINPATH + "results/temp.csv"
MAIN_RESULTS = BINPATH + "results/main.csv"



# results for each NUM_RUNS runs
run_results = []

# read values for the runs in temp file
def get_run_results():
    res = []
    with open(TEMP, 'r') as file:
        reader = csv.reader(file, delimiter=",")    
        for row in reader:
        # print("\nPrice: ",float(row[0]))
            res.append(float(row[0]))

    return res



# get median value from list


# process and write results to main results file
def write_results(results,num_runs):
    with open(MAIN_RESULTS, "a", newline='') as res_file:
        writer = csv.writer(res_file,delimiter=',')        
        writer.writerow([num_runs,statistics.median(results)])
    results.clear()
        



# read current iteration
#def get_cur_iter():
#    with open(LOSS, "r") as file:
#        # read last line
#        for row in reversed(list(csv.reader(file, delimiter=","))):
#            return int(row[0])

def clean(filename):
    if os.path.exists(filename):
        os.remove(filename) 
    else:
        print(f'{filename} does not exist..')
     




# run program (paldb)
def run_bench(): 
    # remove previous bench results
    clean(MAIN_RESULTS)    
    # remove temp file
    clean(TEMP)
    print("--------------Running specjvm bench--------------------")
   
    #do warm up
    for i in range(WARMUP):
        proc = subprocess.Popen([BINPATH + PROCNAME])
        print(f'----------------Running specjvm warmup: {BENCH_NAME} -----------------')
        proc.wait()
    #
    #run app
    #remove temp results file
    #  
    clean(TEMP)
    
    for i in range(NUM_RUNS):
        proc = subprocess.Popen([BINPATH + PROCNAME])
        print(f'----------------Running specjvm bench. Num keys: {BENCH_NAME} -----------------')
        proc.wait()

    #read run results
    results = get_run_results() 
    print(f'Temp results: {results}')
    write_results(results,str(BENCH_NAME))
    clean(TEMP)
      


run_bench()





