/*
 * Created on Fri Jul 17 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

/* Libc headers */
#undef _SIGNAL_H
#define _LARGEFILE_SOURCE 1
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <limits>
#include <string.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include "ocall_logger.h"

#include <grp.h>

/* SGX headers */
#include "sgx_urts.h"
//#include "../../App.h"
#include "../../Enclave_u.h"
#include <stdio.h>

// for cpuid stuff
#include "../cpuid/graalcpuid.h"

void ocall_dlsym(void *handle, const char *symbol, void *res)
{
    log_ocall(__func__);
    printf("Dlsym symbol: %s\n", symbol);
    res = dlsym(handle, symbol);
}

void *ocall_dlopen(const char *filename, int flag)
{
    log_ocall(__func__);
    return dlopen(filename, flag);
}

int ocall_dlclose(void *handle)
{
    log_ocall(__func__);
    return dlclose(handle);
}

void *ocall_dlmopen(long lmid, const char *filename, int flags)
{
    log_ocall(__func__);
    return dlmopen((Lmid_t)lmid, filename, flags);
}

int ocall_dlinfo(void *handle, int request, void *info)
{
    log_ocall(__func__);
    return dlinfo(handle, request, info);
}

char *ocall_dlerror()
{
    log_ocall(__func__);
    return dlerror();
}



long ocall_sysconf(int name)
{
    log_ocall(__func__);
    return sysconf(name);
}

unsigned int ocall_getuid()
{
    log_ocall(__func__);
    uid_t ret = getuid();
    return (unsigned int)ret;
}

void ocall_getcwd(char *buf, size_t size)
{
    log_ocall(__func__);
    getcwd(buf, size);
}

void *ocall_getpwuid(uid_t uid)
{
    log_ocall(__func__);
    // ret = getpwuid(uid);
    return (void *)getpwuid(uid);
}
void ocall_exit(int status)
{
    log_ocall(__func__);
    exit(status);
}

int ocall_getrlimit(int res, struct rlimit *rlim)
{
    log_ocall(__func__);
    return getrlimit(res, rlim);
}
int ocall_setrlimit(int resource, struct rlimit *rlim)
{
    log_ocall(__func__);
    // if (resource == 7) return 0;
    printf("ocall_setrlimit(resource: %d; rlim->rlim_cur: %lu; rlim->rlim_max: %lu)\n",
           resource, rlim->rlim_cur, rlim->rlim_max);
    return setrlimit(resource, rlim);
}

int ocall_uname(struct utsname *buf)
{
    log_ocall(__func__);
    return uname(buf);
}

unsigned int ocall_sleep(unsigned int secs)
{
    log_ocall(__func__);
    return sleep(secs);
}

void ocall_realpath(const char *path, char *res_path)
{
    log_ocall(__func__);
    realpath(path, res_path);
}

void ocall_xpg_strerror_r(int errnum, char *buf, size_t buflen)
{
    log_ocall(__func__);
    char err[8] = "error";
    buf = err;
}

/* Signals */

extern "C"
{
    void break_out();
}
void break_out()
{
}
int ocall_sigaction_graal(int signum, const void *act, void *oldact)
{
    // log_ocall(__func__);
    break_out();
    return sigaction(signum, (struct sigaction *)act, (struct sigaction *)oldact);
}

int ocall_sigemptyset_graal(sigset_t *set)
{
    log_ocall(__func__);
    return sigemptyset(set);
}

int ocall_sigaddset_graal(sigset_t *set, int signum)
{
    log_ocall(__func__);
    return sigaddset(set, signum);
}

__sighandler_t ocall_signal_graal(int signum, __sighandler_t handler)
{
    log_ocall(__func__);
    return nullptr; // signal(signum, handler);
}

/* Mem management */
int ocall_munmap(void *addr, size_t len)
{
    log_ocall(__func__);
    return munmap(addr, len);
}

void *ocall_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    log_ocall(__func__);
    return mmap(addr, length, prot, flags, fd, offset);
}
int ocall_mprotect(void *addr, size_t len, int prot)
{
    // int new_prot = prot;
    log_ocall(__func__);
    return mprotect(addr, len, prot);
}

/* cpuid: for libchelper.a */
#include <cpuid.h>
/* Part 1: for trusted side: enclave */

unsigned int ocall_get_cpuid_max(unsigned int ext, unsigned int *sig)
{
    log_ocall(__func__);
    // return __get_cpuid_max(ext, sig);
    return get_cpuid_max(ext, sig);
}
int ocall_get_cpuid_count(unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    log_ocall(__func__);
    return get_cpuid_count(leaf, subleaf, eax, ebx, ecx, edx);
    /* int cpuInfo[4];
    asm volatile("cpuid"
                 : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
                 : "a"(leaf), "c"(0));

    *eax = cpuInfo[0];
    *ebx = cpuInfo[1];
    *ecx = cpuInfo[2];
    *edx = cpuInfo[3];

    return 1; */
}
int ocall_get_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    log_ocall(__func__);
    return get_cpuid(leaf, eax, ebx, ecx, edx);
    // return (get_cpuid_count(leaf, 0, eax, ebx, ecx, edx));
}

/* Part 2: for untrusted side: app */
// TODO: put these prototypes in a header file
#if defined(__cplusplus)
extern "C"
{
#endif

    char *SVM_FindJavaTZmd(const char *tzmappings, int length);

#if defined(__cplusplus)
}
#endif

char *SVM_FindJavaTZmd(const char *tzmappings, int length)
{
    char *ret = "dummy_tz";
    return ret;
}

int ocall_dladdr(const void *addr, void *info)
{
    log_ocall(__func__);
    return dladdr(addr, (Dl_info *)info);
}
int ocall_kill(int pid, int sig)
{
    log_ocall(__func__);
    return kill((pid_t)pid, sig);
}
int ocall_getgrgid_r(unsigned int gid, void *grp, char *buf, size_t buflen, void **result)
{
    log_ocall(__func__);
    return getgrgid_r((gid_t)gid, (struct group *)grp, buf, buflen, (struct group **)result);
}

int ocall_getpwuid_r(unsigned int uid, void *pwd, char *buf, size_t buflen, void **result)
{
    log_ocall(__func__);
    return getpwuid_r((uid_t)uid, (struct passwd *)pwd, buf, buflen, (struct passwd **)result);
}

int ocall_deflateReset(Z_STREAMP stream)
{
    log_ocall(__func__);
    // TODO: deflateReset is defined in zlib: https://github.com/madler/zlib/blob/master/zlib.h
}
