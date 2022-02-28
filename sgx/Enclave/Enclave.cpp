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

#include "Enclave.h"

// Graal headers
#include "graal_isolate.h"
#include "main.h"

/* Global variables */
sgx_enclave_id_t global_eid;
bool enclave_initiated;
graal_isolatethread_t *global_enc_iso;

char **environ;

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

/**
 * Create global enclave isolate to service ecalls.
 */
void ecall_create_enclave_isolate()
{
    printf("Example of function ptr in the enclave: %p\n", &ecall_create_enclave_isolate);

    int ret;
    printf(">>>>>>>>>>>>>>>>>>> Creating global enclave isolate ...\n");
    global_enc_iso = isolate_generator();
    //destroy_isolate(enc_iso);
    //enc_iso2 = isolate_generator();
    //destroy_isolate(enc_iso2);
    //graal_isolatethread_t *temp = isolate_generator();
    //destroy_isolate(temp);
    printf(">>>>>>>>>>>>>>>>>>> Global enclave isolate creation successfull!\n");
    //printf(">>>>>>>>>>>>>>>>>>> isolate destruction...\n");
    //destroy_isolate(enc_iso);
    //printf(">>>>>>>>>>>>>>>>>>> OK!\n");
}

/**
 * Destroy global enclave isolate
 */
void ecall_destroy_enclave_isolate()
{
    destroy_isolate(global_enc_iso);
}

/**
 * Set environment pointer
 */

void ecall_set_environ(void **env_ptr)
{
    set_environ(env_ptr);
}
/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

void fill_array()
{
    printf("Filling inside array\n");
    unsigned int size = 1024 * 1024 * 4; //16mb
    int *array = (int *)malloc(sizeof(int) * size);
    int idx = 0;
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
        idx = i;
    }
    printf("Largest index in: %d\n", idx);
}

//run main w/0 args: default
void ecall_graal_main(int id)
{
    global_eid = id;
    enclave_initiated = true;
    global_enc_iso = isolate_generator();
    printf("<<<<<< Ecall graal main >>>>>>\n");

    char str[16];
    snprintf(str, 16, "%d", 1000); //good
    //creating GC arguments
    char *argv[16] = {str, "-XX:+PrintGC", "-XX:+VerboseGC"};   

    run_main(1,NULL);
    run_main(3, argv);
}

//run main with an additional argument
void ecall_graal_main_args(int id, int arg1)
{
    global_eid = id;
    enclave_initiated = true;
    //global_enc_iso = isolate_generator();
    printf("In ecall graal main w/ args: %d\n", arg1);

    char str[32];
    snprintf(str, 32, "%d", arg1); //good
    //creating GC arguments
    //char *argv[32] = {"run_main", str, "-XX:+PrintGC", "-XX:+VerboseGC"};
    //run_main(4, argv);

    char *argv[32] = {"run_main", str};
    run_main(2, argv);
}

void *graal_job(void *arg)
{
    //int sum = graal_add(enc_iso, 1, 2);
    //printf("Enclave Graal add 1+2 = %d\n", sum);

    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx Native Image Code Start xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    run_main(1, NULL);

    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  Native Image Code End  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
}