/*
 * Created on Wed Aug 17 2022
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

/* Libc headers */
#undef _SIGNAL_H
#define _LARGEFILE_SOURCE 1

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <map>
#include <unistd.h>
#include "ocall_logger.h"

/* SGX headers */
#include "sgx_urts.h"
//#include "../../App.h"
#include "../../Enclave_u.h"
#include <stdio.h>

pthread_mutex_t handler_map_mutex;
static std::map<int, unsigned long int> signal_to_handler_id_info_map;

extern sgx_enclave_id_t global_eid;

int ocall_sigemptyset(sigset_t *set)
{
	log_ocall(__func__);
	return sigemptyset(set);
}

int ocall_sigfillset(sigset_t *set)
{
	log_ocall(__func__);
	return sigfillset(set);
}

int ocall_sigaddset(sigset_t *set, int signo)
{
	log_ocall(__func__);
	return sigaddset(set, signo);
}

int ocall_sigdelset(sigset_t *set, int signo)
{
	log_ocall(__func__);
	return sigdelset(set, signo);
}

int ocall_sigismember(const sigset_t *set, int signo)
{
	log_ocall(__func__);
	return sigismember(set, signo);
}

int ocall_sigsuspend(const sigset_t *set)
{
	log_ocall(__func__);
	return sigsuspend(set);
}

int ocall_sigaction(int sig, struct sigaction *act, struct sigaction *oact)
{
	log_ocall(__func__);
	return sigaction(sig, act, oact);
}

int ocall_sigpending(sigset_t *set)
{
	log_ocall(__func__);
	return sigpending(set);
}

int ocall_sigwait(const sigset_t *set, int *sig)
{
	log_ocall(__func__);
	return sigwait(set, sig);
}

int ocall_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	log_ocall(__func__);
	return sigprocmask(how, set, oldset);
}

bool get_handler(int signo, unsigned long int *handler_id)
{
	log_ocall(__func__);
	// Retreive the job information for the corresponding job id
	//  printf("Some one call get_handler %d \n", job_id);
	std::map<int, unsigned long int>::iterator it = signal_to_handler_id_info_map.find(signo);
	if (it != signal_to_handler_id_info_map.end())
	{
		unsigned long int *tmp = &it->second;
		*handler_id = *tmp;
		return true;
	}
	else
	{
		return false;
	}
}

void *call_signal_handler(void *arg)
{
	log_ocall(__func__);
	unsigned long int *handler_id = (unsigned long int *)arg;

	// int enclave_id = *handler_id%100;
	usleep(10);
	int errorcode = ecall_generic_signal_handler(global_eid, *handler_id);
	if (errorcode != 0)
	{
		printf("Ecall errocode return is: %x \n", errorcode);
	}
	else
	{
		// printf("Done the handler with this handler id: %lu \n", *handler_id);
	}
}

void sgx_generic_sig_handler(int signo)
{
	unsigned long int handler_id;
	log_ocall(__func__);
	if (signo == SIGTERM || signo == SIGILL)
	{
		// abort();
		exit(0);
	}

	if (get_handler(signo, &handler_id))
	{
		/* Invoke ecall using different thread because SGX doesn't allow nested ecall-ocall-ecall */
		pthread_t tid;
		pthread_create(&tid, NULL, call_signal_handler, &handler_id);
		/* wait for the thread to finish and then return  */
		(void)pthread_join(tid, NULL);
	}
}

__sighandler_t ocall_signal(int __sig, __sighandler_t __handler)
{
	log_ocall(__func__);
	__sighandler_t ret = signal(__sig, __handler);
	if (ret == SIG_ERR)
	{
		printf("Error signal, reason is: %s \n", strerror(errno));
	}
	// printf("ocall_signal: %x \n", __sig);
	return ret;
}

__sighandler_t ocall_signal_generic(int __sig, unsigned long __handler)
{
	log_ocall(__func__);
	unsigned long int handler_id = __handler;
	pthread_mutex_lock(&handler_map_mutex);
	signal_to_handler_id_info_map.insert(std::pair<int, unsigned long int>(__sig, handler_id));
	pthread_mutex_unlock(&handler_map_mutex);
	__sighandler_t ret = signal(__sig, &sgx_generic_sig_handler);

	if (ret == SIG_ERR)
	{
		printf("Error signal, reason is: %s \n", strerror(errno));
	}

	// abort();
	// printf("ocall_signal_generic: %x \n", __sig);
	return ret;
}

int ocall_sigaction_generic(int sig, struct sigaction *act, struct sigaction *oact)
{
	log_ocall(__func__);
	unsigned long int handler_id = (unsigned long int)act->sa_handler;
	pthread_mutex_lock(&handler_map_mutex);
	signal_to_handler_id_info_map.insert(std::pair<int, unsigned long int>(sig, handler_id));
	pthread_mutex_unlock(&handler_map_mutex);
	act->sa_handler = &sgx_generic_sig_handler;
	int ret = sigaction(sig, act, oact);

	if (ret < 0)
	{
		printf("Error signal, reason is: %s \n", strerror(errno));
	}

	// abort();
	// printf("ocall_signal_generic: %x \n", __sig);
	return ret;
}

int ocall_raise(int sig)
{
	log_ocall(__func__);
	return raise(sig);
}
