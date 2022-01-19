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
NUM_RUNS = 5 #10


PROCNAME = "app"
BINPATH = "./"
TEMP = BINPATH + "results/temp.csv"

MAIN_RESULTS = BINPATH + "results/main.csv"

SEC_PERCENT = sys.argv[1]

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



# get mean/median value from list


# process and write results to main results file
def write_results(results):
    with open(MAIN_RESULTS, "a", newline='') as res_file:
        writer = csv.writer(res_file,delimiter=',')        
        writer.writerow([SEC_PERCENT,statistics.mean(results)])
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
     

def create_temp():
    if os.path.exists(TEMP):
        print('Temp file exists')
    else:
        open(TEMP,"x")


# run program (paldb)
def run_bench(): 
    # remove previous bench results
    #clean(MAIN_RESULTS)    
    # remove temp file
      
  
    #do warm up
    for i in range(WARMUP):
        proc = subprocess.Popen([BINPATH + PROCNAME])
        print(f'----------------Running simulator bench warmup-----------------')
        proc.wait()
    #
    #run app
    #remove temp results file
    # 
    clean(TEMP)
   

    
    #create_temp()
    for i in range(NUM_RUNS):
        proc = subprocess.Popen([BINPATH + PROCNAME])
        print(f'----------------Running simulator bench-----------------')
        proc.wait()

    #read run results
    results = get_run_results() 
    print(f'Temp results: {results}')       
    clean(TEMP)
    write_results(results)

run_bench()





