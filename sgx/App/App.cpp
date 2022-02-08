/*
 * Created on Fri Jul 17 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

/*
 * Copyright (C) 2011-2019 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#define MAX_PATH FILENAME_MAX

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

//#define ____sigset_t_defined
#define __iovec_defined 1

#include "Enclave_u.h"
#include "sgx_urts.h"

#include "App.h"
#include "error/error.h"

// Graal headers
#include "graal_isolate.h"
#include "main.h"
#include "user_types.h"

/* Signal handlers */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <map>
#include "ocall_logger.h"

/* Benchmarking */
//#include "benchtools.h"
#include <time.h>
struct timespec start, stop;
double diff;
using namespace std;
extern std::map<pthread_t, pthread_attr_t *> attr_map;

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

/* Main app isolate */
graal_isolatethread_t *global_app_iso;
/*Main thread id*/
//std::thread::id main_thread_id = std::this_thread::get_id();
pthread_t main_thread_id;

/* Ocall counter */
unsigned int ocall_count = 0;
std::map<std::string, int> ocall_map;

/* external variables */
extern char **environ;

void gen_sighandler(int sig, siginfo_t *si, void *arg)
{
    printf("Caught signal: %d\n", sig);
}

/**
 * Generates isolates.
 * This can be used to generate execution contexts for transition routines.
 */

graal_isolatethread_t *isolate_generator()
{
    graal_isolatethread_t *temp_iso = NULL;
    int ret;
    if ((ret = graal_create_isolate(NULL, NULL, &temp_iso)) != 0)
    {
        printf("Error on app isolate creation or attach. Error code: %d\n", ret);

        return NULL;
    }
    return temp_iso;
}

/**
 * Destroys the corresponding isolates.
 */

void destroy_isolate(graal_isolatethread_t *iso)
{

    if (graal_tear_down_isolate(iso) != 0)
    {
        printf("Isolate shutdown error\n");
    }
}

void fill_array()
{
    printf("Filling outside array\n");
    unsigned int size = 1024 * 1024 * 4; //16mb
    int *array = (int *)malloc(sizeof(int) * size);
    int idx = 0;
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
        idx = i;
    }
    printf("Largest index is %d\n", idx);
}

/**
 * Set main thread attribs
 */
void setMainAttribs()
{
    main_thread_id = pthread_self();
    pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
    int ret = pthread_getattr_np(main_thread_id, attr);
    attr_map.insert(pair<pthread_t, pthread_attr_t *>(main_thread_id, attr));
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    //run_main(1, NULL);
    //return 0;
    //I use only 1 arg for now
    int arg1 = 0;

    //const char* arg1 = argv[1];

    global_app_iso = isolate_generator();

    printf("<<<<<<< Created untrusted app isolate >>>>>> \n");
    getchar();//pyuhala: halt to verify message b4 the clumsy debug messages

    setMainAttribs();

    attr_map.insert(pair<pthread_t, pthread_attr_t *>(0, NULL));

    /* Initialize the enclave */
    if (initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
    printf("Enclave initialized\n");

    int id = global_eid;

    if (argc > 1)
    {
        arg1 = atoi(argv[1]);
        ecall_set_environ(global_eid, (void **)environ);
        ecall_graal_main_args(global_eid, id, arg1);
    }
    else
    {
        ecall_set_environ(global_eid, (void **)environ);
        ecall_graal_main(global_eid, id);
    }

    //ecall_create_enclave_isolate(global_eid);
    //ecall_graal_main_args(global_eid, id, arg1);
    /**
     * Invoke main routine of java application: for partitioned apps. 
     * This is the initial entrypoint method, all further ecalls are performed there.
     */

    //run_main(argc, argv);

    printf("Number of ocalls: %d\n", ocall_count);
    showOcallLog(10);
    writeVal("./results/temp.csv", ocall_count);

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    /*  if (graal_tear_down_isolate(iso_thread) != 0)
    {
        printf("isolate shutdown error\n");
    }
 */
    /*  printf("Time inside: %lf\n", in);
    printf("Time outside: %lf\n", out); */

    //printf("Enter a character before exit ...\n");
    //getchar();
    return 0;
}
