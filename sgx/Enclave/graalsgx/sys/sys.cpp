/*
 * Created on Fri Jul 17 2020
 *
 * Copyright (c) 2020 Peterson Yuhala and Jämes Ménétrey, IIUN
 */

#include <sgx/mem/sgx_mman.h>
#include <errno.h>
#include "checks.h"  //for pointer checks
#include "Enclave.h" //for printf
#include "graalsgx_malloc.h"
//#include "sgx_rsrv_mem_mngr.h"

static int map_array[NUM_MAPS];

/*We do not support dynamic loading of libs. We get the appropriate routine name and call the wrapper function.*/
void *getSymbolHandle(const char *symbol)
{
    //TODO: add diff return vals for diff symbols
    if (strcmp(symbol, "inet_pton") == 0)
    {
        return (void *)&inet_pton;
    }
    else if (strcmp(symbol, "openat64") == 0)
    {
        return (void *)&open;
    }
    else if (strcmp(symbol, "") == 0)
    {
        return (void *)&inet_pton;
    }

    else if (strcmp(symbol, "") == 0)
    {
        return (void *)&inet_pton;
    }

    else if (strcmp(symbol, "") == 0)
    {
        return (void *)&inet_pton;
    }

    else if (strcmp(symbol, "") == 0)
    {
        return (void *)&inet_pton;
    }

    else if (strcmp(symbol, "") == 0)
    {
        return (void *)&inet_pton;
    }
}

void *dlsym(void *handle, const char *symbol)
{

    GRAAL_SGX_INFO();
    void *res = getSymbolHandle(symbol);
    printf("Symbol: %s\n", symbol);
    //ocall_dlsym(handle, symbol, res);
    return res;
}

void *dlopen(const char *filename, int flag)
{
    GRAAL_SGX_INFO();
    void *res = nullptr;
    ocall_dlopen(&res, filename, flag);
    return res;
}

long sysconf(int name)
{
    GRAAL_SGX_INFO();
    long ret;
    ocall_sysconf(&ret, name);
    return ret;
}

uid_t getuid()
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_getuid(&ret);
    return (uid_t)ret;
}

char *getcwd(char *buf, size_t size)
{
    GRAAL_SGX_INFO();
    ocall_getcwd(buf, size);
    return buf;
}

struct passwd *getpwuid(uid_t uid)
{
    GRAAL_SGX_INFO();
    struct passwd *ret;
    ocall_getpwuid(uid, ret);
    return ret;
}
void exit(int status)
{
    GRAAL_SGX_INFO();
    ocall_exit(status);
}

int getrlimit(int res, struct rlimit *rlim)
{
    GRAAL_SGX_INFO();
    int ret;
    //printf("Resource limit b4: cur = %ld, max = %ld\n", rlim->rlim_cur, rlim->rlim_max);
    ocall_getrlimit(&ret, res, rlim);
    //printf("Resource limit after: resource = %d cur = %ld, max = %ld\n", res, rlim->rlim_cur, rlim->rlim_max);
    return ret;
}

int setrlimit(int resource, const struct rlimit *rlim)
{
    GRAAL_SGX_INFO();
    int ret;
    //TODO
    printf("Resource limit set: res: %d cur = %ld, max = %ld\n", resource, rlim->rlim_cur, rlim->rlim_max);
    return 0;
    ocall_setrlimit(&ret, resource, (struct rlimit *)rlim);
    return ret;
}

long syscall(long num, ...)
{
    GRAAL_SGX_INFO();
    long ret = 0;
    //ocall_syscall(...)
    return ret;
}

int uname(struct utsname *buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_uname(&ret, buf);
    return ret;
}

unsigned int sleep(unsigned int secs)
{
    GRAAL_SGX_INFO();
    unsigned int ret;
    ocall_sleep(&ret, secs);
    return ret;
}

char *realpath(const char *path, char *res_path)
{
    GRAAL_SGX_INFO();
    ocall_realpath(path, res_path);
    return res_path;
}

char *__xpg_strerror_r(int errnum, char *buf, size_t buflen)
{
    GRAAL_SGX_INFO();
    ocall_xpg_strerror_r(errnum, buf, buflen);
    return buf;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    GRAAL_SGX_INFO();
    int ret;
    //TODO
    //ocall_sigaction(&ret, signum, act, oldact);
    return ret;
}

int sigemptyset(sigset_t *set)
{
    GRAAL_SGX_INFO();
    int ret;
    //TODO
    //ocall_sigemptyset(&ret, set);
    return ret;
}
int sigaddset(sigset_t *set, int signum)
{
    GRAAL_SGX_INFO();
    int ret;
    //TODO
    //ocall_sigaddset(&ret, set, signum);
    return ret;
}
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    GRAAL_SGX_INFO();
    int ret;
    //TODO
    //ocall_sigprocmask(&ret, how, set, oldset);
    return ret;
}
__sighandler_t signal(int signum, __sighandler_t handler)
{
    GRAAL_SGX_INFO();
    //TODO
    //printf("Signal num: %d\n", signum);
    //handler = &sig_handler;
    __sighandler_t ret = &sig_handler; //nullptr;
    //ocall_signal(&ret, signum, handler);
    return nullptr;
}

void sig_handler(int param)
{
    GRAAL_SGX_INFO();
}
/* Mem management */

/**
 * Unmaps an area of memory within the SGX Reserved Memory.
 */
int munmap(void *addr, size_t len)
{
    uint64_t page_size, aligned_length;

    GRAAL_SGX_DEBUG_PRINTF("munmap called; addr: %p, len: %ld", addr, len);

    // Align the length according to the size of the memory pages
    page_size = getpagesize();
    aligned_length = (len + page_size - 1) & ~(page_size - 1);

    // Unmap the memory
    sgx_free_rsrv_mem(addr, aligned_length);
}

/**
 * Maps an area of memory within the SGX Reserved Memory.
 */
void *mmap(void *hint, size_t length, int prot, int flags, int fd, off_t offset)
{

    uint64_t page_size, aligned_length;
    void *aligned_hint, *reserved_memory_ptr;
    int memory_protection_flags = 0;
    sgx_status_t status;

    GRAAL_SGX_DEBUG_PRINTF("mmap called fd; %d prot: %d flags: %d size: %d hint: %p offset: %ld", fd, prot, flags, length, hint, offset);

    // Do not support memory mapping of file or device.
    // File mapping can be added by reading the content of the file and writing it into the memory area.
    if (fd != -1)
    {
        //TODO: use mmap64 in this case
        errno = EBADF;
        GRAAL_SGX_DEBUG_PRINT("error: mmap cannot map a file or a device in the enclave.");
        return MAP_FAILED;
    }

    // Align the hint and the length according to the size of the memory pages
    page_size = getpagesize();
    aligned_length = (length + page_size - 1) & ~(page_size - 1);
    aligned_hint = (void *)((((size_t)hint) + page_size - 1) & ~(page_size - 1));

    // Allocate the memory
    reserved_memory_ptr = sgx_alloc_rsrv_mem(aligned_length);
    if (reserved_memory_ptr == NULL)
    {
        errno = ENOMEM;
        GRAAL_SGX_DEBUG_PRINT("error: the memory allocation failed.");
        return MAP_FAILED;
    }

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
        errno = EACCES;
        GRAAL_SGX_DEBUG_PRINT("error: the protection of the allocated memory could not be set.");
        return MAP_FAILED;
    }

    GRAAL_SGX_DEBUG_PRINTF("mmap successfully allocated memory at address: %p", reserved_memory_ptr);

    return reserved_memory_ptr;
}

/**
 * Change the protection of an area of memory within the SGX Reserved Memory.
 */
int mprotect(void *addr, size_t len, int prot)
{
    uint64_t page_size, aligned_length;
    int memory_protection_flags = 0;
    sgx_status_t status;

    GRAAL_SGX_DEBUG_PRINTF("mprotect called; addr: %p, len: %ld, prot: %d", addr, len, prot);

    // Align the length according to the size of the memory pages
    page_size = getpagesize();
    aligned_length = (len + page_size - 1) & ~(page_size - 1);

    // Maps the POSIX protection flags with the SGX flags
    if (prot & MMAP_PROT_READ)
        memory_protection_flags |= SGX_PROT_READ;
    if (prot & MMAP_PROT_WRITE)
        memory_protection_flags |= SGX_PROT_WRITE;
    if (prot & MMAP_PROT_EXEC)
        memory_protection_flags |= SGX_PROT_EXEC;

    // Change the protection of the memory
    status = sgx_tprotect_rsrv_mem(addr, aligned_length, memory_protection_flags);

    if (status != SGX_SUCCESS)
    {
        GRAAL_SGX_DEBUG_PRINT("error: the protection of the memory could not be set.");
        errno = EACCES;
        return -1;
    }

    return 0;
}

/* cpuid routines: for libchelper.a */
unsigned int get_cpuid_max(unsigned int ext, unsigned int *sig)
{
    GRAAL_SGX_INFO();
    unsigned int ret;
    ocall_get_cpuid_max(&ret, ext, sig);
    //printf("cpu max level is: %d--------------------------------\n", *sig);
    return ret;

    /* return __get_cpuid_max(ext, sig); */
}

int get_cpuid_count(unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    GRAAL_SGX_INFO();

    int ret;
    ocall_get_cpuid_count(&ret, leaf, subleaf, eax, ebx, ecx, edx);
    return ret;

    //return 1;
    /* __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
    return 1; */
}

int get_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
    GRAAL_SGX_INFO();
    /* int ret;
    ocall_get_cpuid(&ret, leaf, 0, eax, ebx, ecx, edx);
    return ret; */

    return (get_cpuid_count(leaf, 0, eax, ebx, ecx, edx));
}


pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
} //out
pid_t vfork(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
}
pid_t fork(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
}
int statvfs64(const char *path, struct statvfs *buf)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
} //out
int execve(const char *pathname, char *const argv[], char *const envp[])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
}
int execvp(const char *file, char *const argv[])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    //TODO
    return ret;
}
void _exit(int status)
{
    sgx_exit();
}

int dladdr(const void *addr, void *info)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_dladdr(&ret, addr, info);
    return ret;
}
int kill(int pid, int sig)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_kill(&ret, pid, sig);
    return ret;
}
int getgrgid_r(unsigned int gid, void *grp, char *buf, size_t buflen, void **result)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getgrgid_r(&ret, gid, grp, buf, buflen, result);
    return ret;
}
int getpwuid_r(unsigned int uid, void *pwd, char *buf, size_t buflen, void **result)
{

    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getpwuid_r(&ret, uid, pwd, buf, buflen, result);
    return ret;
}
int deflateReset(Z_STREAMP stream)
{

    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_deflateReset(&ret, stream);
    return ret;
}
