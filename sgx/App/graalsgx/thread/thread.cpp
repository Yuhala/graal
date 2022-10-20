/*
 * Created on Fri Jul 24 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 *
 */

#include <thread>
#include <pthread.h>
#include "Enclave_u.h"
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include "sgx_eid.h"
#include "../../error/error.h"
#include "struct/sgx_pthread_struct.h"
#include "ocall_logger.h"
#include <map>

#define GRAAL_SGX_STACK 0x40000UL // 256kb

extern sgx_enclave_id_t global_eid;
using namespace std;

std::map<pthread_t, pthread_attr_t *> attr_map;
std::map<pthread_t *, pthread_t *> thread_in_out_map;

/* #define MAX_PTHREAD_ATTR 50000
pthread_attr_t *attr_array[MAX_PTHREAD_ATTR];
static int count = 0; */

typedef struct
{
    unsigned long int job_id;
    sgx_enclave_id_t eid;
} internal_ecall_arg;

// forward declarations
int manual_setstack(pthread_attr_t *attr, void **stk_addr, size_t *stack_size);

void *generic_ecall_routine(void *arguments)
{

    GRAAL_SGX_INFO();
    internal_ecall_arg *ecall_arg = (internal_ecall_arg *)arguments;
    printf("Job id: %d, enclave eid: %d\n", ecall_arg->job_id, ecall_arg->eid);
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_execute_job(global_eid, pthread_self(), ecall_arg->job_id);
    print_error_message(ret);
    free(arguments);
}

int ocall_pthread_attr_init(void *_attr)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();

    /*
    pthread_t id = TEMP_ID;
    attr_array[++count] = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
    int ret = pthread_attr_init(attr_array[count]);
    *attr = ret < 0 ? PTHREAD_ATTRIBUTE_NULL : count;
    return ret; */
    return 0;
}

int ocall_pthread_condattr_setclock(void *_attr, clockid_t clock_id, size_t attr_len)
{
    log_ocall(__func__);
    pthread_condattr_t *attr = (pthread_condattr_t *)_attr;
    return pthread_condattr_setclock(attr, clock_id);
}

int ocall_pthread_create(pthread_t *new_thread_in, unsigned long int job_id, sgx_enclave_id_t eid)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    internal_ecall_arg *arguments = (internal_ecall_arg *)malloc(sizeof(internal_ecall_arg));
    arguments->job_id = job_id;
    arguments->eid = global_eid; // eid; //pyuhala: our tests for now use one enclave so this should be the right value.

    /* Create attrib */
    pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
    // increase stack size

    pthread_t new_thread_out;
    int ret = pthread_create(&new_thread_out, NULL, generic_ecall_routine, (void *)arguments);
    // int ret = pthread_create(new_thread, NULL, generic_ecall_routine, (void *)arguments);
    pthread_attr_t *new_thread_attr;
    // int attr_ret = pthread_getattr_np(*new_thread, new_thread_attr);
    // pthread_attr_setstacksize(attr, GRAAL_SGX_STACK);

    // pthread_attr_setstacksize(attr, GRAAL_SGX_STACK);
    /* Add tid and attr pair to map */
    // attr_map.insert(pair<pthread_t, pthread_attr_t *>(*new_thread, new_thread_attr));

    thread_in_out_map.insert(pair<pthread_t *, pthread_t *>(new_thread_in, &new_thread_out));
    attr_map.insert(pair<pthread_t, pthread_attr_t *>(*new_thread_in, attr));

    void *val;
    printf("Calling thread id: %d\n", pthread_self());
    printf("Created thread id: %d\n", new_thread_out);
    //pthread_join(new_thread_out, &val);

    return ret;
}

pthread_attr_t *getAttrib(pthread_t tid)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    pthread_attr_t *attr = NULL;
    auto it = attr_map.find(tid);
    if (it != attr_map.end())
    {
        attr = it->second;
    }
    return attr;
}

pthread_t ocall_pthread_self(void)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    return pthread_self();
}

int ocall_pthread_join(pthread_t pt, void **res)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    return pthread_join(pt, NULL);
}

int ocall_pthread_attr_getguardsize(size_t *guardsize)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    int ret = -1;

    pthread_attr_t *attr = getAttrib(pthread_self());
    {
        size_t size;
        ret = pthread_attr_getguardsize(attr, &size);
        *guardsize = size;
    }
    return ret;
}

int ocall_pthread_attr_destroy()
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    int ret = -1;
    pthread_attr_t *attr = getAttrib(pthread_self());
    if (attr != NULL)
    {
        ret = pthread_attr_destroy(attr);
        free(attr);
    }

    attr_map[pthread_self()] = NULL;
    return ret;
}

int ocall_pthread_attr_getstack__bypass(void *attr, size_t attr_len, void **stk_addr, size_t len, size_t *stack_size, pthread_t id)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();

    pthread_attr_t *attrib = getAttrib(pthread_self());

    int ret;

    ret = pthread_attr_getstack((pthread_attr_t *)attr, stk_addr, stack_size);
    return ret;

    if (attrib == NULL)
    {
        printf("!!!!!!!!!!!>>>>>>>>>> thread attribute does not exist/NULL: %s ...\n", __FUNCTION__);
    }
    else
    {
        ret = pthread_attr_getstack((pthread_attr_t *)attrib, stk_addr, stack_size);
        printf("getstack outside: pid: %d stk_addr: %p pthread_attr_t: %p stack_size: %d>>>>>>>\n", id, *stk_addr, attrib, *stack_size);
    }

    if (*stack_size == 0)
    {
        // ret = manual_setstack((pthread_attr_t *)attrib, stk_addr, stack_size);
    }

    return ret;
}

int ocall_pthread_attr_getstack(void **stk_addr, size_t *stack_size)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    int ret = -1;
    pthread_attr_t *attr = getAttrib(pthread_self());
    if (attr != NULL)
    {
        printf(">>>>>>> thread attribute found\n");
        size_t size;
        void **addr;
        ret = pthread_attr_getstack(attr, stk_addr, stack_size);
        // printf("Stack addr out: %p\n", *stk_addr);
        //*stack_size = size;
        //*stk_addr = *addr;
        printf("pthread_attr_getstack ret vals: stk_addr: %p, stk_size: %d", *stk_addr, *stack_size);
    }

    if (*stack_size == 0)
    {
        ret = manual_setstack(attr, stk_addr, stack_size);
    }
    return ret;
}

/**
 * @brief
 * Pyuhala:
 * Manually set the threads stack attributes.
 * Based on code from: https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-pthread-attr-setstack-set-stack-attribute
 *
 * @param stk_addr
 * @param stack_size
 * @return int
 */
int manual_setstack(pthread_attr_t *attr, void **stk_addr, size_t *stack_size)
{
    // set stack size
    *stack_size = 3 * PTHREAD_STACK_MIN;
    // init attributes

    /**
     * @brief
     * Get a big enough stack and align on 4K boundary.
     *
     */
    *stk_addr = malloc(PTHREAD_STACK_MIN * 3);
    if (*stk_addr != NULL)
    {
        *stk_addr = (void *)((((long)*stk_addr + (PTHREAD_STACK_MIN - 1)) /
                              PTHREAD_STACK_MIN) *
                             PTHREAD_STACK_MIN);
    }
    else
    {
        perror("Manual malloc stack allocation failed.");
        exit(2);
    }
    printf(">>>>> Setting stackaddr to %x\n", *stk_addr);
    printf(">>>>>> Setting stacksize to %x\n", *stack_size);
    int rc = pthread_attr_setstack(attr, stk_addr, *stack_size);
    if (rc != 0)
    {
        printf("pthread_attr_setstack returned: %d\n", rc);
        exit(3);
    }

    return rc;
}

int ocall_pthread_getattr_np(pthread_t tid)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    int ret = -1;
    // printf("Attrib tid: %d \n", tid);
    pthread_attr_t *attr = getAttrib(tid);
    // pthread_t thread = (pthread_t)attr;
    if (attr != NULL)
    {
        ret = pthread_getattr_np(tid, attr);
    }
    else
    {
        printf(">>>>>>>>>>>>>>>>>> thread attr does not exist in list\n");
    }
    return ret;
}

int ocall_pthread_getattr_np__bypass(pthread_t tid, void *attr, size_t len)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();

    int ret = pthread_getattr_np(tid, (pthread_attr_t *)attr);
    return ret;
}

int ocall_pthread_attr_getguardsize__bypass(void *attr, size_t attr_len, size_t *guardsize)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    return pthread_attr_getguardsize((pthread_attr_t *)attr, guardsize);
}

int ocall_pthread_attr_destroy__bypass(void *attr, size_t attr_len)
{
    log_ocall(__func__);
    GRAAL_SGX_INFO();
    return pthread_attr_destroy((pthread_attr_t *)attr);
}