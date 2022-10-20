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

// for pwd_test
#define _POSIX_SOURCE
#include <sys/types.h>
#include <pwd.h>

/* Benchmarking */
#include "bench/benchtools.h"

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
// std::thread::id main_thread_id = std::this_thread::get_id();
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

// zlib test
int gun_main(int argc, char **argv);

/**
 * @brief
 * From IBM code examples.
 */
void pwd_test()
{
    struct passwd *p;
    uid_t uid = getuid();

    if ((p = getpwuid(uid)) == NULL)
        perror("getpwuid() error");
    else
    {
        printf("getpwuid() returned the following info for uid %d:\n",
               (int)uid);
        printf("  pw_name  : %s\n", p->pw_name);
        printf("  pw_uid   : %d\n", (int)p->pw_uid);
        printf("  pw_gid   : %d\n", (int)p->pw_gid);
        printf("  pw_dir   : %s\n", p->pw_dir);
        printf("  pw_shell : %s\n", p->pw_shell);
    }
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
 * Create global enclave isolate to service ecalls.
 */
void create_app_isolate()
{
    // printf("Example of function ptr in the enclave: %p\n", &ecall_create_enclave_isolate);

    int ret;
    printf(">>>>>>>>>>>> Creating global app isolate >>>>>>>>>>>>...\n");
    if ((ret = graal_create_isolate(NULL, NULL, &global_app_iso)) != 0)
    {
        printf("Error on app isolate creation or attach. Error code: %d\n", ret);
        exit(1);
    }
    else
    {
        printf(">>>>>>>>> Global app isolate creation successfull! >>>>>>>>>>>>>\n");
    }
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
    unsigned int size = 1024 * 1024 * 4; // 16mb
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

void *get_stack_ptr()
{
    // intptr_t sp;
    // asm("movq %%rsp, %0"
    //     : "=r"(sp));

    register void *sp asm("sp");
    return (void *)sp;
}

void stack_addr_test()
{
    pthread_t thid;
    void *sp = get_stack_ptr();
    pthread_attr_t *global_attr;
    global_attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
    // increase stack size

    /* Initialize attr */
    int rc = pthread_attr_init(global_attr);
    if (rc != 0)
    {
        printf("error attr_init");
    }

    void *stack_addr;
    size_t stack_size;
    /* Get the default stack_addr and stack_size value */
    rc = pthread_attr_getstack(global_attr, &stack_addr, &stack_size);
    if (rc != 0)
    {
        printf("error attr getstack");
    }
    printf("rsp = %p, stack_addr = %p, stack_size = %u\n", sp, stack_addr, stack_size);
}

/* Application entry */
int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    // I use only 1 arg for now
    int arg1 = 0;
    bool no_sgx = false;
    bool partitioned_app = true;

    // global_app_iso = isolate_generator();
    create_app_isolate();

    if (no_sgx)
    {
        start_clock(&start);
        run_main(1, NULL);
        stop_clock(&stop);

        printf(">>>>>>>>>>>>>>> Total run time: %f s >>", time_diff(&start, &stop, SEC));
        return 0;
    }

    // graal_isolatethread_t *temp = isolate_generator();

    setMainAttribs();

    attr_map.insert(pair<pthread_t, pthread_attr_t *>(0, NULL));

    /* Initialize the enclave */
    if (initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }

    int geid = global_eid;
    printf("Enclave initialized. EID: %d >>>>>>> \n", global_eid);

    ecall_set_environ(global_eid, geid, (void **)environ);
    ecall_create_enclave_isolate(global_eid);

    // run_main(1, NULL);

    // if (argc > 1)
    // {
    //     arg1 = atoi(argv[1]);

    //     ecall_graal_main_args(global_eid, id, arg1);
    // }
    // else
    // {

    //     ecall_graal_main(global_eid, id);
    // }

    /**
     * Invoke main routine of java application: for partitioned apps.
     * This is the initial entrypoint method, all further ecalls are performed there.
     */

    /**
     * @brief
     * Partitioned programs have the main entry point in the untrusted runtime.
     * Call run_main to launch such programs.
     */

    start_clock(&start);
    run_main(argc, argv);
    stop_clock(&stop);

    /**
     * @brief
     * Call ecall_graal_main for programs run fully inside the enclave.
     *
     */
    // start_clock(&start);
    // ecall_graal_main(global_eid, geid);
    // stop_clock(&stop);

    printf("<<<<<<<<<<<<<<<<<<<  >>>>>>>>>>>>>>>>>>>>>>>>> Total run time: %f s >>", time_diff(&start, &stop, SEC));

    showOcallLog(10);
    // writeVal("./results/temp.csv", ocall_count);

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    /*  if (graal_tear_down_isolate(iso_thread) != 0)
    {
        printf("isolate shutdown error\n");
    }
 */
    /*  printf("Time inside: %lf\n", in);
    printf("Time outside: %lf\n", out); */

    // printf("Enter a character before exit ...\n");
    // getchar();
    return 0;
}
