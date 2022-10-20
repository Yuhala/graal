/*
 * Created on Fri Mar 06 2020
 *
 * Copyright (c) 2020 Peterson Yuhala
 */
#include "benchtools.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * register start timestamp in start variable
 */
void start_clock(struct timespec *start)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, start);
}
/**
 * register stop time in stop variable;
 * will be used to compute diff with a start var;
 */
void stop_clock(struct timespec *stop)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, stop);
}

/**
 * get elapsed time in secs since start
 */
double elapsed_time(struct timespec *start)
{
    struct timespec stop; // creates stop on the stack
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    return time_diff(start, &stop, SEC);
}

/* Calculates mean of array of values */
double get_mean(std::vector<double> &v)
{

    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    return sum / v.size();
}
// Function to swap two pointers
void swap(double *a, double *b)
{
    double temp = *a;
    *a = *b;
    *b = temp;
}

void quicksort(std::vector<double> &target, int left, int right)
{
    if (left >= right)
        return;
    int i = left, j = right;
    double tmp, pivot = target[i];
    for (;;)
    {
        while (target[i] < pivot)
            i++;
        while (pivot < target[j])
            j--;
        if (i >= j)
            break;
        tmp = target[i];
        target[i] = target[j];
        target[j] = tmp;
        i++;
        j--;
    }
    quicksort(target, left, i - 1);
    quicksort(target, j + 1, right);
}

/* Calculate median of array of values */
double get_median(std::vector<double> &data)
{
    // Q0:0 Q1:1 Q2:2(median) Q3:3
    // Do pointer checks first

    int index = MEDIAN_INDEX;
    quicksort(data, 0, NUM_ITERATIONS - 1);
    // printf("Min: %f Max: %f\n", data[0], data[NUM_ITERATIONS - 1]);
    return data[index];
}

/* Calculates standard deviation of array of values */
double get_SD(std::vector<double> &data)
{

    double mean = 0.0, sd = 0.0;
    mean = get_mean(data);

    for (int i = 0; i < data.size(); i++)
    {
        sd += pow(data[i] - mean, 2);
    }
    return sqrt(sd / data.size());
}

double get_tput(int num_ops, double time)
{
    return (1.0 * num_ops) / time;
}
/* Calculates time diff between two timestamps in m */

double time_diff(timespec *start, timespec *stop, granularity gran)
{

    double diff = 0.0;

    switch (gran)
    {
    case MILLI:
        diff = (double)(stop->tv_sec - start->tv_sec) * 1.0e3 + (double)((stop->tv_nsec - start->tv_nsec) / 1.0e6);
        break;

    case MICRO:
        diff = (double)(stop->tv_sec - start->tv_sec) * 1.0e6 + (double)((stop->tv_nsec - start->tv_nsec) / 1.0e3);
        break;

    case NANO:
        diff = (double)(stop->tv_sec - start->tv_sec) * 1.0e9 + (double)((stop->tv_nsec - start->tv_nsec));
        break;

    case SEC: // seconds
        diff = (double)(stop->tv_sec - start->tv_sec) + (double)((stop->tv_nsec - start->tv_nsec) / 1.0e9);
        break;
    }
    return diff;
}


void register_results(const char *path, int numKeys, double runTime)
{
    FILE *fptr = fopen(path, "ab+");
    fprintf(fptr, "%d, %f\n", numKeys, runTime);
    fclose(fptr);
}

/**
 * Some benchmark specific routines.
 */
void register_results(const char *path, int numKeys, double runTime, double tput, double cpu)
{

    FILE *fptr = fopen(path, "ab+");
    fprintf(fptr, "%d, %f, %f, %f\n", numKeys, runTime, tput, cpu);
    fclose(fptr);
}

