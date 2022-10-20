/**
 * @author ${kekkaishivn} - dattl@ifi.uio.no
 *
 * ${tags}
 */

#include <map>
#include <sgx_trts.h>
#include "checks.h" //for pointer checks
#include "../../Enclave.h"

sgx_thread_mutex_t handler_map_mutex = SGX_THREAD_MUTEX_INITIALIZER;
std::map<unsigned long int, __sighandler_t> id_to_handler_info_map;
extern sgx_enclave_id_t enclave_eid;
sgx_enclave_id_t enclave_self_id = enclave_eid;

extern bool enclave_initiated;

bool initiated_self_id = enclave_initiated;

// forward declarations
bool get_handler(unsigned long int job_id, __sighandler_t *pt_job);
unsigned long int put_handler(__sighandler_t new_handler, int __sig);

static inline unsigned long int get_an_random_id(int __sig)
{
    unsigned long int rand_number = 0;
    sgx_read_rand((unsigned char *)(&rand_number), sizeof(rand_number));

    printf("Initiated self id: %d Enclave self id: %d >>>>>>>>>>>\n", (int)initiated_self_id, (int)enclave_self_id);
    if (!initiated_self_id || enclave_self_id >= 100)
    {
        // abort();
        // printf("Initiated self id: %d Enclave self id: %d >>>>>>>>>>>\n", (int)initiated_self_id, (int)enclave_self_id);
    }

    // Reserved 100 for the eid
    return rand_number - (rand_number % 10000) + enclave_self_id + 100 * __sig;
}

void ecall_generic_signal_handler(unsigned long int handler_id)
{
    GRAAL_SGX_INFO();
    // debug("Call ecall_generic_signal_handler");
    __sighandler_t generic_handler;
    if (get_handler(handler_id, &generic_handler))
    {
        int signo = (handler_id % 10000) / 100;
        // sgx_wrapper_printf("Handler address: %p - with signo: %d \n", generic_handler, signo);
        generic_handler(signo);
    }
}

bool get_handler(unsigned long int job_id, __sighandler_t *pt_job)
{
    GRAAL_SGX_INFO();
    // Retreive the job information for the corresponding job id
    //  printf("Some one call get_handler %d \n", job_id);
    std::map<unsigned long int, __sighandler_t>::iterator it = id_to_handler_info_map.find(job_id);
    if (it != id_to_handler_info_map.end())
    {
        __sighandler_t *tmp = &it->second;
        *pt_job = *tmp;
        return true;
    }
    else
    {
        return false;
    }
}

unsigned long int put_handler(__sighandler_t new_handler, int __sig)
{
    
    GRAAL_SGX_INFO();
    unsigned long int new_id = get_an_random_id(__sig);
    sgx_thread_mutex_lock(&handler_map_mutex);
    while (id_to_handler_info_map.count((new_id)) > 0)
    {
        new_id = get_an_random_id(__sig);
    }
    id_to_handler_info_map.insert(std::pair<unsigned long int, __sighandler_t>(new_id, new_handler));
    sgx_thread_mutex_unlock(&handler_map_mutex);
    return new_id;
}

int sigemptyset(sigset_t *set)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigemptyset(&retval, set);
    CHECK_STATUS(status);
    return retval;
}

int sigfillset(sigset_t *set)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigfillset(&retval, set);
    CHECK_STATUS(status);
    return retval;
}

int sigaddset(sigset_t *set, int signo)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigaddset(&retval, set, signo);
    CHECK_STATUS(status);
    return retval;
}

int sigdelset(sigset_t *set, int signo)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigdelset(&retval, set, signo);
    CHECK_STATUS(status);
    return retval;
}

int sigismember(const sigset_t *set, int signo)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigismember(&retval, set, signo);
    CHECK_STATUS(status);
    return retval;
}

int sigsuspend(const sigset_t *set)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigsuspend(&retval, set);
    CHECK_STATUS(status);
    return retval;
}

int sigaction(int sig, struct sigaction *act, struct sigaction *oact)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status;
    __sighandler_t __handler = act->sa_handler;
    if (__handler != SIG_DFL && __handler != SIG_IGN && __handler != SIG_ERR)
    {
        // debug("Handler is: %p \n", __handler);
        unsigned long int handler_id = put_handler(__handler, sig);
        act->sa_handler = (__sighandler_t)handler_id;
        status = ocall_sigaction_generic(&retval, sig, act, oact);
    }
    else
    {
        // debug("ocall_signal Handler is: %p \n", __handler);
        status = ocall_sigaction(&retval, sig, act, oact);
    }
    CHECK_STATUS(status);
    return retval;
}

int sigpending(sigset_t *set)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigpending(&retval, set);
    CHECK_STATUS(status);
    return retval;
}

int sigwait(const sigset_t *set, int *sig)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_sigwait(&retval, set, sig);
    CHECK_STATUS(status);
    return retval;
}

__sighandler_t signal(int __sig, __sighandler_t __handler)
{
    GRAAL_SGX_INFO();
    __sighandler_t retval;
    // sgx_status_t status;
    // if (__handler != SIG_DFL && __handler != SIG_IGN && __handler != SIG_ERR)
    // {
    //     // debug("Handler is: %p \n", __handler);
    //     unsigned long int handler_id = put_handler(__handler, __sig);
    //     status = ocall_signal_generic(&retval, __sig, handler_id);
    // }
    // else
    // {
    //     // debug("ocall_signal Handler is: %p \n", __handler);
    //     status = ocall_signal(&retval, __sig, __handler);
    // }
    // CHECK_STATUS(status);
    // return retval;
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    GRAAL_SGX_INFO();
    int ret;
    sgx_status_t status;
    // TODO
    status = ocall_sigprocmask(&ret, how, set, oldset);
    CHECK_STATUS(status);
    return ret;
}

int raise(int signal)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_raise(&retval, signal);
    CHECK_STATUS(status);
    return retval;
}