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

/* Global variables */
sgx_enclave_id_t global_eid;
bool enclave_initiated;
graal_isolatethread_t *global_enc_iso;

__thread uintptr_t thread_stack_address;

char **environ;

//------------------------------- stack fxns -----------------------------
//#include "stack.h"

#define CALL_STACK_MAXlEN 64

#define STACK_HIGH 0XFFFFFFFF

// forward declarations
void *switch_builtin(unsigned int level);

void set_stack_address()
{
    // thread_stack_address = __builtin_frame_address(0);

    register uintptr_t sp asm("sp");
    thread_stack_address = sp;
    printf("Setting stack pointer SP: 0x%016 \n", sp);
}

void *get_stack_address(unsigned int level)
{
    // not sure why level as input causes compilation errors
    // return __builtin_frame_address(0);
    return (void *)thread_stack_address;
}

void *get_stack_ptr()
{
    // intptr_t sp;
    // asm("movq %%rsp, %0"
    //     : "=r"(sp));

    register void *sp asm("sp");
    return (void *)sp;
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
void *get_stack_bottom()
{
    void *before = __builtin_frame_address(0); // just an initial value
    void *curr = __builtin_return_address(0);
    unsigned int level = 0;

    // traverse the stack addresses until u reach top (i.e ret = 0)
    int a = 0;
    before = curr = (void *)&a;

    /* while (level < CALL_STACK_MAXlEN && curr != 0 && false)
    {
        before = curr;
        curr = switch_builtin(level);
        printf("get_stack_top::curr: %p >>>>>>>\n", curr);
        level++;
    } */

    while (curr < (void *)STACK_HIGH)
    {
        curr = curr + 0x10;
        before = curr;
    }
    return before;
}

/**
 * @brief
 * C doesn't accept variable as input to __builtinxx
 * So we need a large switch case tree. We end with 20 tests for now.
 *
 * @param level
 * @return void*
 */
void *switch_builtin(unsigned int level)
{
    switch (level)
    {
    case 0:
        return __builtin_return_address(0);
        break;
    case 1:
        return __builtin_return_address(1);
        break;
    case 2:
        return __builtin_return_address(2);
        break;
    case 3:
        return __builtin_return_address(3);
        break;
    case 4:
        return __builtin_return_address(4);
        break;

    case 5:
        return __builtin_return_address(5);
        break;

    case 6:
        return __builtin_return_address(6);
        break;

    case 7:
        return __builtin_return_address(7);
        break;

    case 8:
        return __builtin_return_address(8);
        break;

    case 9:
        return __builtin_return_address(9);
        break;

    case 10:
        return __builtin_return_address(10);
        break;

    case 11:
        return __builtin_return_address(11);
        break;

    case 12:
        return __builtin_return_address(12);
        break;

    case 13:
        return __builtin_return_address(13);
        break;

    case 14:
        return __builtin_return_address(14);
        break;

    case 15:
        return __builtin_return_address(15);
        break;

    case 16:
        return __builtin_return_address(16);
        break;

    case 17:
        return __builtin_return_address(17);
        break;

    case 18:
        return __builtin_return_address(8);
        break;

    case 19:
        return __builtin_return_address(19);
        break;

    case 20:
        return __builtin_return_address(20);
        break;

    default:
        return __builtin_return_address(20);
        break;
    }
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
    printf("Example of function ptr in the enclave: %p\n", &ecall_create_enclave_isolate);

    int ret;
    printf(">>>>>>>>>>>>>>>>>>> Creating global enclave isolate ...\n");
    global_enc_iso = isolate_generator();
    // destroy_isolate(enc_iso);
    // enc_iso2 = isolate_generator();
    // destroy_isolate(enc_iso2);
    // graal_isolatethread_t *temp = isolate_generator();
    // destroy_isolate(temp);
    printf(">>>>>>>>>>>>>>>>>>> Global enclave isolate creation successfull!\n");
    // printf(">>>>>>>>>>>>>>>>>>> isolate destruction...\n");
    // destroy_isolate(enc_iso);
    // printf(">>>>>>>>>>>>>>>>>>> OK!\n");
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
    // set_stack_address();
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
    unsigned int size = 1024 * 1024 * 4; // 16mb
    int *array = (int *)malloc(sizeof(int) * size);
    int idx = 0;
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
        idx = i;
    }
    printf("Largest index in: %d\n", idx);
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

    global_eid = id;
    enclave_initiated = true;
    global_enc_iso = isolate_generator();
    printf("============================= Ecall graal main: global_enc_iso = %p\n", (void *)global_enc_iso);

    char str[16];
    snprintf(str, 16, "%d", 1000); // good
    // creating GC arguments
    char *argv[16] = {str, "-XX:+PrintGC", "-XX:+VerboseGC"};

    printf("============================= Entering run_main =========================\n");

    // printf("polytaint_add result: %d >>>>>>>>>>>>>>>>>\n", polytaint_add(global_enc_iso, 44, 33));
    enclave_create_context(global_enc_iso);
    // gc_test(global_enc_iso, 1000);

    // set stack address just before entering java code
    set_stack_address();
    run_main(1, NULL);
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
    global_eid = id;
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