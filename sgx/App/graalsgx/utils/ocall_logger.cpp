/*
 * Created on Tue Mar 23 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 * Defines routines used to count ocall transitions
 */

#include "ocall_logger.h"
#include "stdlib.h"
#include "stdio.h"
#include <bits/stdc++.h>
#include <map>

using namespace std;
extern std::map<std::string, int> ocall_map;
extern unsigned int ocall_count;

// Comparator function to sort pairs in ascending order
bool cmp(pair<string, int> &a,
         pair<string, int> &b)
{
    return a.second > b.second;
}

void log_ocall(const char *func)
{
    ocall_count++;
    std::string name = std::string(func);

    if (ocall_map.find(name) != ocall_map.end())
    {
        // kv pair exists in map
        ocall_map[name] += 1;
    }
    else
    {
        // kv pair does not exist yet
        ocall_map.insert(std::make_pair(name, 1));
    }

    // printf("Calling ocall is: %s\n", name);
}

/**
 * Print top num frequent ocalls
 */
void showOcallLog(int num)
{
    printf("------- OCALL STATS --------\n");
    printf("------- Total ocalls: %d\n", ocall_count);
    // create vector with map kv pairs
    vector<pair<string, int>> vect;

    for (auto &it : ocall_map)
    {
        vect.push_back(it);
    }
    // printf("size of vect is: %d\n",vect.size());
    // sort vector
    sort(vect.begin(), vect.end(), cmp);
    int count = 0;
    // print first num elements in the vector
    printf("--------- Top %d ---------------------------\n", num);
    for (auto &it : vect)
    {
        printf("Ocall: %s Count: %d\n", it.first.c_str(), it.second);
        count++;
        if (count >= num)
        {
            break;
        }
    }
}

void writeVal(const char *path, int val)
{
    FILE *fptr;
    fptr = fopen(path, "a");

    if (fptr == NULL)
    {
        printf("Error opening file!");
        exit(1);
    }
    fprintf(fptr, "%d", val);
    fclose(fptr);
}
