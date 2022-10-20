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

/**
 * @file Enclave.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-06-20
 *
 * @copyright Copyright (c) 2022
 * Some useful debugging sites
 * https://www.felixcloutier.com/x86/eresume
 *
 */

#include "Enclave.h"

// Graal headers
#include "graal_isolate.h"
#include "main.h"

#include "sys/graalsgx_malloc.h"

/* Global variables */
sgx_enclave_id_t enclave_eid = -1;
bool enclave_initiated = false;

graal_isolatethread_t *global_enc_iso;

__thread uintptr_t thread_stack_base;

char **environ;

//------------------------------- stack fxns -----------------------------
//#include "stack.h"

// forward declarations
void *switch_builtin(unsigned int level);

void set_stack_address()
{
    /**
     * @brief
     * PYuhala
     * This method is one of the first called after the
     * initial ecall. We allocate a variable and use it
     * as our stackbase; not exact but is close to the real base.
     *
     */
    unsigned int early_var = 0;

    thread_stack_base = (uintptr_t)get_stack_ptr();
}

void *get_stack_address(unsigned int level)
{
    // not sure why level as input causes compilation errors
    // return __builtin_frame_address(0);
    return (void *)thread_stack_base;
}

void *get_stack_ptr()
{
    // intptr_t sp;
    // asm("movq %%rsp, %0"
    //     : "=r"(sp));

    register void *sp asm("sp");
    return sp;
}

void *get_r15_register()
{
    register void *val asm("r15");
    return val;
}

/**
 * @brief
 * PYuhala
 * Estimating top of the stack by
 * traversing frames until we reach an invalid
 * address.
 *
 * @return void*
 */
void *get_stack_base()
{
    return (void *)thread_stack_base;
}

/**
 * @brief
 * Allocates a buffer to act as thread stack for an enclave
 * thread. The buffer pages are given Read/Write protections.
 * This routine is called by pthread_attr_getstack
 * NB: the real memory allocated will be 4x size_mb
 * @param size_mb
 * @return void*
 */
void *allocate_stack(size_t size_mb)
{
    // give the stack memory read/write prot
    int prot = MMAP_PROT_READ | MMAP_PROT_WRITE;

    unsigned int length = size_mb * 1024 * 1024;

    void *stack_addr;
    size_t stack_size = size_mb * STACK_MIN * 1024;

    uint64_t page_size, aligned_length;
    void *aligned_hint, *reserved_memory_ptr;
    int memory_protection_flags = 0;
    sgx_status_t status;

    // Align the hint and the length according to the size of the memory pages
    page_size = getpagesize();

    // aligned_length = (length + page_size - 1) & ~(page_size - 1);
    // aligned_hint = (void *)((((size_t)hint) + page_size - 1) & ~(page_size - 1));

    // Allocate the memory
    stack_addr = sgx_alloc_rsrv_mem(stack_size);
    if (reserved_memory_ptr == NULL)
    {
        GRAAL_SGX_DEBUG_PRINT("error: the stack memory allocation failed.");
    }

    // Align stack on 4K boundary
    stack_addr = (void *)((((long)stack_addr + (STACK_MIN - 1)) / STACK_MIN) * STACK_MIN);

    // Change the protection of the allocated memory
    if (prot & MMAP_PROT_READ)
        memory_protection_flags |= SGX_PROT_READ;
    if (prot & MMAP_PROT_WRITE)
        memory_protection_flags |= SGX_PROT_WRITE;
    if (prot & MMAP_PROT_EXEC)
        memory_protection_flags |= SGX_PROT_EXEC;

    status = sgx_tprotect_rsrv_mem(reserved_memory_ptr, aligned_length, memory_protection_flags);

    if (status != SGX_SUCCESS)
    {
        sgx_free_rsrv_mem(reserved_memory_ptr, aligned_length);
        GRAAL_SGX_DEBUG_PRINT("error: the protection of the allocated stack memory could not be set.");
    }

    return reserved_memory_ptr;
}

//---------------------------------------------------------

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
    // printf("Example of function ptr in the enclave: %p\n", &ecall_create_enclave_isolate);

    int ret;
    printf(">>>>>>>>>>>>>>>>>>> Creating global enclave isolate ...\n");
    if ((ret = graal_create_isolate(NULL, NULL, &global_enc_iso)) != 0)
    {
        printf("Error on app isolate creation or attach. Error code: %d\n", ret);
        exit(1);
    }
    else
    {
        printf(">>>>>>>>>>>>>>>>>>> Global enclave isolate creation successfull!\n");
    }
}

/**
 * Destroy global enclave isolate
 */
void ecall_destroy_enclave_isolate()
{
    destroy_isolate(global_enc_iso);
}

/**
 * Set environment variables and some global variables.
 */

void ecall_set_environ(int id, void **env_ptr)
{
    // set_stack_address();
    set_environ(env_ptr);

    enclave_eid = id;
    printf(">>>>>>>>>>>> ecall:seteviron set enclave eid to: %d\n >>>>>>>>>>", enclave_eid);
    enclave_initiated = true;
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

/**
 * @brief
 * Stack overflow test
 */
void repeat();
void repeat()
{
    repeat();
}
/**
 * @brief
 *
 * @param num_allocs
 */
void ecall_stackoverflow_test()
{
    repeat();
    printf("******resuming after stackoverflow: repeat()\n****");
}

// run main w/0 args: default
void ecall_graal_main(int id)
{
    // set_stack_address();
    // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> stack pointer in ecall-graal-main is: %p >>>>>>>>>>\n", (void *)thread_stack_base);

    // global_enc_iso = isolate_generator();
    //  printf("============================= Ecall graal main: global_enc_iso = %p\n", (void *)global_enc_iso);

    char str[16];
    snprintf(str, 16, "%d", 1000); // good
    // creating GC arguments
    char *argv[16] = {str, "-XX:+PrintGC", "-XX:+VerboseGC"};

    printf("============================= Entering run_main =========================\n");

    // set stack address just before entering java code
    run_main(1, NULL);

    return;

    // run_main(3, argv);
}

void ecall_test_pwuid(unsigned int id)
{
    uid_t val = (uid_t)id;
    struct passwd *p = getpwuid(val);
}

// run main with an additional argument
void ecall_graal_main_args(int id, int arg1)
{
    // set_stack_address();
    enclave_eid = id;
    enclave_initiated = true;
    // global_enc_iso = isolate_generator();
    printf("In ecall graal main w/ args: %d\n", arg1);

    char str[32];
    snprintf(str, 32, "%d", arg1); // good
    // creating GC arguments
    // char *argv[32] = {"run_main", str, "-XX:+PrintGC", "-XX:+VerboseGC"};
    // run_main(4, argv);

    char *argv[32] = {"run_main", str};
    run_main(2, argv);
}

void *graal_job(void *arg)
{
    // int sum = graal_add(enc_iso, 1, 2);
    // printf("Enclave Graal add 1+2 = %d\n", sum);

    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx Native Image Code Start xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    run_main(1, NULL);

    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  Native Image Code End  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
}