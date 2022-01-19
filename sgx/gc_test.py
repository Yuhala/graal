#!/usr/bin/env python

#
# Author: Peterson Yuhala
# Graal-SGX Benchmarking script
#

#/substratevm/src/com.oracle.svm.core.genscavenge/src/com/oracle/svm/core/genscavenge/GCImpl.java

import os
import sys
import subprocess
import time
import csv
import math
import statistics
from random import randint
import re

# number of warm up runs
WARMUP = 1
# number of runs
NUM_RUNS = 10


PROCNAME = "app"
BINPATH = "./"
TEMP = BINPATH + "results/temp.csv"
MAIN_RESULTS = BINPATH + "results/main.csv"

LOG = BINPATH + "results/log.txt"



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
     


# obtains useful info from log file
# this method specifically returns the last number in the GraalVM GC log: ie total gc time
# This method can easily be adapted to return the other useful values
def read_log(filename):
    with open(filename) as file:
        text = file.read()
    # test print
    #print(f'LOG text: {text}')
    # read all numbers from the file
    gcVals = re.findall('[0-9]+',text)
    #print(f'GC vals list: {gcVals}')
    # this represents the total gc time in nanosecs
    totalGCTime = int(gcVals[len(gcVals)-1])
    #print(f'Total GC time: {totalGCTime} ns')

    value = str(totalGCTime) + "\n"
    # append its val to the temp file
    tmpFile = open(TEMP,'a')      
    tmpFile.write(value)
    tmpFile.close()





# run program (paldb)
def run_bench():    

    # remove previous bench results
    clean(MAIN_RESULTS)    
    # remove temp file
    clean(TEMP)
    print("--------------Running GC bench------------------")
    max_objs = 100000
    num_objs = 10000
    #do warm up
    
    for i in range(WARMUP):
        proc = subprocess.Popen([BINPATH + PROCNAME,str(num_objs)])
        print(f'----------------Running GC test warmup. Num keys: {num_objs} -----------------')
        proc.wait()


    #
    #run app
    #remove temp results file
    #   
       
    
    clean(TEMP)
    while(num_objs <= max_objs):
        for i in range(NUM_RUNS):
             # this file will contain stderr (eg java log output)
            f = open(LOG, "w")
            print(f'----------------Running GC test bench. Num keys: {num_objs} -----------------')
            proc = subprocess.Popen([BINPATH + PROCNAME,str(num_objs)],stderr=f)                           
            proc.wait()

            read_log(LOG) 
            f.close()
            clean(LOG) 
            

        #read run results
        results = get_run_results() 
        print(f'Temp results: {results}')
        write_results(results,str(num_objs))
        clean(TEMP)
        num_objs += 10000


run_bench()





