/*
 * Created on Fri Mar 06 2020
 *
 * Copyright (c) 2020 Peterson Yuhala
 */

#ifndef BENCHTOOLS_H
#define BENCHTOOLS_H

/* includes */
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <numeric>

/* For benchmarking */

#define NUM_ITERATIONS (10 + 1) // Default:100 added 1 so as to get middle number for calculating median
#define NUM_POINTS 64           // Default:100 32000 points approx= 64MB...
#define WARM_UP 10              // Default:10
#define SHIFT 1 << 19           // 1<<16 //Equals 256KB for 4byte elts eg ints
#define MEDIAN_INDEX (NUM_ITERATIONS / 2)
#define Q1_INDEX (MEDIAN_INDEX / 2)                // starting index for 1st quartile
#define Q3_INDEX (NUM_ITERATIONS - (Q1_INDEX + 1)) // starting index for 3rd quartile

/**
 * sometimes we have many data points.
 * The register results funtion can use this value to skip
 * some points
 */
#define POINT_FREQ 1

typedef enum
{
    MILLI,
    MICRO,
    NANO,
    SEC
} granularity;

void quicksort(std::vector<double> &vect, int left, int right);
double get_median(std::vector<double> &vect);
double get_mean(std::vector<double> &vect);
void swap(double *a, double *b);
double get_SD(std::vector<double> &vect);
double time_diff(timespec *start, timespec *stop, granularity gran);
void start_clock(struct timespec *start);
void stop_clock(struct timespec *stop);
double elapsed_time(struct timespec *start);
void register_results(const char *path, int numKeys, double runTime);
void register_results(const char *path, int numKeys, double runTime, double tput);
void register_results(const char *path, int numKeys, double runTime, double tput, double cpu);
void register_results_dynamic(const char *path, double timestamp, double req_tput, unsigned int num_workers, double cpu_usage);

double get_tput(int num_ops, double time);

#endif /* BENCHTOOLS_H */
