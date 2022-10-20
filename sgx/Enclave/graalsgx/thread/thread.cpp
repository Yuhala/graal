/*
 * Created on Fri Jul 24 2020
 *
 * Copyright (c) 2017 Panoply
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 * Some code ideas here is based on code from Panoply source code e.g job map
 *
 */
#include "../../checks.h" //for pointer checks
#include "../../Enclave.h"
#include <map>
#include <sgx_trts.h>

extern __thread uintptr_t thread_stack_address;

extern sgx_enclave_id_t enclave_eid;
extern bool enclave_initiated;

sgx_thread_mutex_t job_map_mutex = SGX_THREAD_MUTEX_INITIALIZER;
sgx_thread_mutex_t pthreadid_map_mutex = SGX_THREAD_MUTEX_INITIALIZER;
std::map<unsigned long int, pthread_job_t> id_to_job_info_map;
std::map<sgx_thread_t, pthread_t> sgx_thread_id_to_pthread_id_info_map;

int pthread_create(pthread_t *thread, GRAAL_SGX_PTHREAD_ATTR attr,
                   void *(*start_routine)(void *), void *arg)
{
    GRAAL_SGX_INFO();
    if (!enclave_initiated)
    {
        fprintf(SGX_STDERR, "The enclave has not been initiated.");
        abort();
    }
    

    pthread_job_t new_job = {start_routine, arg};
    unsigned long int job_id = put_job(new_job);
    int ret;
    ocall_pthread_create(&ret, thread, job_id, enclave_eid);
    return ret;
}

pthread_t pthread_self(void)
{
    GRAAL_SGX_INFO();
    pthread_t ret;
    ocall_pthread_self(&ret);
    return ret;
}

int pthread_join(pthread_t thread, void **retval)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_pthread_join(&ret, thread, retval);
    return ret;
}

/*int pthread_attr_getguardsize(GRAAL_SGX_PTHREAD_ATTR attr, size_t *guardsize)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_pthread_attr_getguardsize(&ret, guardsize);
    return ret;
}*/

int nanosleep(const struct timespec *__requested_time, struct timespec *__remaining)
{
    // TODO
    return 0;
}

int sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask)
{
    // TODO
    return 0;
}
int sched_getaffinity(pid_t pid, size_t cpusetsize, void *mask)
{
    // TODO
    return 0;
}

int __sched_cpucount(size_t setsize, const void *setp)
{
    // TODO
    return 0;
}

int pthread_attr_setdetachstate(GRAAL_SGX_PTHREAD_ATTR attr, int detachstate)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret = 0;
    return ret;
}
int pthread_attr_init(void *attr)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret = 0;
    return ret;
}
int pthread_setname_np(pthread_t thread, const char *name)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret = 0;
    return ret;
}
int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret = 0;
    return ret;
}
int pthread_attr_setstacksize(GRAAL_SGX_PTHREAD_ATTR attr, size_t stacksize)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret = 0;
    return ret;
}

int pthread_attr_getguardsize(GRAAL_SGX_PTHREAD_ATTR attr, size_t *guardsize)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_pthread_attr_getguardsize__bypass(&ret, attr, sizeof(pthread_attr_t), guardsize);
    // ocall_pthread_attr_getguardsize(&ret, guardsize);
    return ret;
}

int pthread_attr_destroy(GRAAL_SGX_PTHREAD_ATTR attr)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_pthread_attr_destroy__bypass(&ret, attr, sizeof(pthread_attr_t));
    // ocall_pthread_attr_destroy(&ret);
    return ret;
}

int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
{
    GRAAL_SGX_INFO();
    // printf("[ENCLAVE] pthread_attr_getstack(..)\n");

    int ret = 0;
    pthread_t id;
    //*stackaddr = (void *)get_stack_base();
    //*stacksize = 0x1000000;

    //*stackaddr = allocate_stack(128);
    //*stacksize = 512 * 1024 * 1024;

    ocall_pthread_self(&id);
    ocall_pthread_attr_getstack__bypass(&ret, attr, sizeof(pthread_attr_t), stackaddr, sizeof(intptr_t), stacksize, id);
    // ocall_pthread_attr_getstack(&ret, stackaddr, stacksize);
    printf(">>>>>>>>>> attr_get_stack details in enclave:: pid: %lu stackaddr: %p stack-size: %d\n", id, *stackaddr, *stacksize);
    return ret;
}

/**
 * @brief
 * PYuhala:
 * custom reimplementation of function
 * @param attr
 * @param stackaddr
 * @param stacksize
 * @return int
 */

int xxxxpthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
{
    GRAAL_SGX_INFO();
}

int xxxpthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    intptr_t sp; // stack pointer
                 // printf("Dummy stack addr: %p\n", *stackaddr);
                 //*stackaddr = get_stack_bottom();//(void *)0x00007f7bd0;
                 //*stacksize = 0x800000;
    ocall_pthread_attr_getstack(&ret, stackaddr, stacksize);
    printf(">>>>>>>>>> attr_get_stack details in enclave:: stackaddr: %p stack-size: %d\n", *stackaddr, *stacksize);

    return ret;
}

int pthread_getattr_np(pthread_t tid, GRAAL_SGX_PTHREAD_ATTR attr)
{
    GRAAL_SGX_INFO();
    printf("[ENCLAVE] pthread_getattr_np(tid: %lu, attr: %p)\n", tid, attr);
    printf(">>>>>>>>>> SGX thread id: %lu\n", sgx_thread_self());
    printf(">>>>>>>>>> POSIX thread id: %lu\n", pthread_self());
    int ret = 0;
    ocall_pthread_getattr_np__bypass(&ret, tid, attr, sizeof(pthread_attr_t));
    // ocall_pthread_getattr_np(&ret, tid);
    return ret;
}

/*int pthread_getattr_np(pthread_t tid, GRAAL_SGX_PTHREAD_ATTR attr)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_pthread_getattr_np(&ret, tid);
    return ret;
}*/

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
    GRAAL_SGX_INFO();
    // TODO xxx
    int ret = 0;
    ret = ocall_pthread_condattr_setclock(&ret, attr, clock_id, sizeof(pthread_condattr));

    return ret;
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    
    memset(attr, '\0', sizeof(*attr)); // TODO: allocate attr in map outside with pthread id
    // ocall_pthread_condattr_init(&ret, attr);
    return ret;
}

int pthread_cond_timedwait(pthread_cond_t *__restrict__ cond,
                           pthread_mutex_t *__restrict__ mutex,
                           const struct timespec *__restrict__ abstime)
{
    // GRAAL_SGX_INFO();
    int ret = 0;
    // TODO xxx
    ret = sgx_thread_cond_wait((sgx_thread_cond_t *)cond, (sgx_thread_mutex_t *)mutex);
    return ret;
}

int pthread_mutex_init(pthread_mutex_t *__restrict__ mutex, const pthread_mutexattr_t *__restrict__ attr)
{
    return sgx_thread_mutex_init((sgx_thread_mutex_t *)mutex, (sgx_thread_mutexattr_t *)attr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{

    return sgx_thread_mutex_lock((sgx_thread_mutex_t *)mutex);
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    return sgx_thread_mutex_trylock((sgx_thread_mutex_t *)mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    return sgx_thread_mutex_unlock((sgx_thread_mutex_t *)mutex);
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    return sgx_thread_cond_broadcast((sgx_thread_cond_t *)cond);
}

int pthread_cond_wait(pthread_cond_t *__restrict__ cond, pthread_mutex_t *__restrict__ mutex)
{
    return sgx_thread_cond_wait((sgx_thread_cond_t *)cond, (sgx_thread_mutex_t *)mutex);
}

int pthread_cond_init(pthread_cond_t *__restrict__ cond, const pthread_condattr_t *__restrict__ attr)
{
    return sgx_thread_cond_init((sgx_thread_cond_t *)cond, (sgx_thread_condattr_t *)attr);
}

int sched_yield(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    Pause();
    return ret;
}

/**
 *  Job related functions
 *  Copyright (c) Panoply 2017
 */

static inline unsigned long int get_random_id()
{
    unsigned long int rand_number = 0;
    sgx_read_rand((unsigned char *)(&rand_number), sizeof(rand_number));
    return rand_number;
}

unsigned long int put_job(pthread_job_t new_job)
{
    unsigned long int new_id = get_random_id();
    sgx_thread_mutex_lock(&job_map_mutex);
    while (id_to_job_info_map.count((new_id)) > 0)
    {
        new_id = get_random_id();
    }
    id_to_job_info_map.insert(std::pair<unsigned long int, pthread_job_t>(new_id, new_job));
    sgx_thread_mutex_unlock(&job_map_mutex);
    return new_id;
}

bool get_job(unsigned long int job_id, pthread_job_t *pt_job)
{
    // Retrieve the job information for the corresponding job id
    //  printf("Some one call get_job %d \n", job_id);
    std::map<unsigned long int, pthread_job_t>::iterator it = id_to_job_info_map.find(job_id);
    if (it != id_to_job_info_map.end())
    {
        pthread_job_t *tmp = &it->second;
        *pt_job = *tmp;
        id_to_job_info_map.erase(job_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool get_pthreadid_from_sgxthreadid(sgx_thread_t sgx_id, pthread_t *pt)
{
    std::map<sgx_thread_t, pthread_t>::iterator it = sgx_thread_id_to_pthread_id_info_map.find(sgx_id);
    if (it != sgx_thread_id_to_pthread_id_info_map.end())
    {
        pthread_t *tmp = &it->second;
        *pt = *tmp;
        return true;
    }
    else
    {
        return false;
    }
}

void ecall_execute_job(pthread_t pthread_id, unsigned long int job_id)
{
    GRAAL_SGX_INFO();
    pthread_job_t execute_job = {NULL, NULL};
    sgx_thread_t sgx_id = sgx_thread_self();

    sgx_thread_mutex_lock(&pthreadid_map_mutex);
    sgx_thread_id_to_pthread_id_info_map.insert(std::pair<sgx_thread_t, pthread_t>(sgx_id, pthread_id));
    sgx_thread_mutex_unlock(&pthreadid_map_mutex);

    if (get_job(job_id, &execute_job))
        if (execute_job.start_routine != NULL)
        {
            printf("Executing start_routine %p by the pthread_id: %d \n", execute_job.start_routine, (unsigned long)pthread_id);
            execute_job.start_routine(execute_job.arg);
        }
}

// Semaphore routines
void sem_init(struct sgx_bsem_t *bsem_p, int value)
{
    if (value < 0 || value > 1)
    {
        printf("Error: sem_init value can take only values 0 and 1");
        exit(1);
    }
    sgx_thread_mutex_init(&(bsem_p->mutex), NULL);
    sgx_thread_cond_init(&(bsem_p->cond), NULL);
    bsem_p->v = value;
}
void sem_reset(struct sgx_bsem_t *bsem_p)
{
    sem_init(bsem_p, 0);
}
void sem_post(struct sgx_bsem_t *bsem_p)
{
    sgx_thread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    sgx_thread_cond_signal(&bsem_p->cond);
    sgx_thread_mutex_lock(&bsem_p->mutex);
}
void sem_post_all(struct sgx_bsem_t *bsem_p)
{
    sgx_thread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    sgx_thread_cond_broadcast(&bsem_p->cond);
    sgx_thread_mutex_unlock(&bsem_p->mutex);
}
void sem_wait(struct sgx_bsem_t *bsem_p)
{
    sgx_thread_mutex_lock(&bsem_p->mutex);
    while (bsem_p->v != 1)
    {
        sgx_thread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
    }
    bsem_p->v = 0;
    sgx_thread_mutex_unlock(&bsem_p->mutex);
}
int sem_destroy(struct sgx_bsem_t* bsem_p){
    //TODO
    return 0;
}
