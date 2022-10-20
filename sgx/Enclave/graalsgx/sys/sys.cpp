/*
 * Created on Fri Jul 17 2020
 *
 * Copyright (c) 2020 Peterson Yuhala and Jämes Ménétrey, IIUN
 */

/**
 * Recommended coding style for ocalls
 *  GRAAL_SGX_INFO();
    xxx retval;
    sgx_status_t status =  ocall_xxx(&retval, xxx);
    CHECK_STATUS(status);
    return retval;
 *
 */

#include <sgx/mem/sgx_mman.h>
#include <errno.h>
#include "checks.h" //for pointer checks
#include "../../Enclave.h"
#include "graalsgx_malloc.h"
#include <sgx/sys/types.h>
//#include "sgx_rsrv_mem_mngr.h"

static int map_array[NUM_MAPS];

// forward declarations
int printDlinfo(void *info, int ret);

//--------------------------------------------------------
/**
 * These functions need to be revisited.
 */
int waitid(int idtype, int id, void *infop, int options)
{
    // TODO
    return 0;
}
int posix_spawn(void *pid, void *path, void *file_actions, void *attrp, void **argv, void **envp)
{
    // TODO
    return 0;
}
int sscanf(const char *str, const char *format)
{
    // TODO
    return 0;
}
ssize_t __getdelim(char **lineptr, size_t *n, int delim, SGX_FILE *stream)
{
    // TODO
    return 0;
}

//--------------------------------------------------------

/*We do not support dynamic loading of libs. We get the appropriate routine name and call the wrapper function.*/
void *getSymbolHandle(const char *symbol)
{
    // TODO: add diff return vals for diff symbols
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
    printf("Symbol is >>>> ----->>>>>: %s\n", symbol);
    // ocall_dlsym(handle, symbol, res);
    return res;
}

void *dlopen(const char *filename, int flag)
{
    GRAAL_SGX_INFO();
    void *res = nullptr;
    ocall_dlopen(&res, filename, flag);
    return res;
}

int dlclose(void *handle)
{

    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_dlclose(&retval, handle);
    CHECK_STATUS(status);
    return retval;
}
int dlinfo(void *handle, int request, void *info)
{
    GRAAL_SGX_INFO();
    int retval;
    sgx_status_t status = ocall_dlinfo(&retval, handle, request, info);
    CHECK_STATUS(status);
    return retval;
}

void *dlmopen(long lmid, const char *filename, int flags)
{
    GRAAL_SGX_INFO();
    void *res = nullptr;
    ocall_dlmopen(&res, lmid, filename, flags);
    return res;
}

char *dlerror()
{
    GRAAL_SGX_INFO();
    char *retval;
    sgx_status_t status = ocall_dlerror(&retval);
    CHECK_STATUS(status);
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
    unsigned int ret;
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
    void *ret;
    ocall_getpwuid(&ret, uid);
    struct passwd *p = (struct passwd *)ret;

    /*if (p == NULL)
    {
        printf("ocall getpwuid() error");
    }

    else
    {
        printf("getpwuid() returned the following info for uid %d:\n",
               (int)uid);
        printf("  pw_name  : %s\n", p->pw_name);
        printf("  pw_uid   : %d\n", (int)p->pw_uid);
        printf("  pw_gid   : %d\n", (int)p->pw_gid);
        printf("  pw_dir   : %s\n", p->pw_dir);
        printf("  pw_shell : %s\n", p->pw_shell);
    }*/

    return p;
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
    // printf("Resource limit b4: cur = %ld, max = %ld\n", rlim->rlim_cur, rlim->rlim_max);
    ocall_getrlimit(&ret, res, rlim);
    // printf("Resource limit after: resource = %d cur = %ld, max = %ld\n", res, rlim->rlim_cur, rlim->rlim_max);
    return ret;
}

int setrlimit(int resource, const struct rlimit *rlim)
{
    GRAAL_SGX_INFO();
    int ret;
    // TODO
    printf("Resource limit set: res: %d cur = %ld, max = %ld\n", resource, rlim->rlim_cur, rlim->rlim_max);
    return 0;
    ocall_setrlimit(&ret, resource, (struct rlimit *)rlim);
    return ret;
}

long syscall(long num, ...)
{
    GRAAL_SGX_INFO();
    long ret = 0;
    // ocall_syscall(...)
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
    printf("realpath pathname: %s\n >>>>>>>>>", path);
    ocall_realpath(path, res_path);
    return res_path;
}

char *__xpg_strerror_r(int errnum, char *buf, size_t buflen)
{
    GRAAL_SGX_INFO();
    ocall_xpg_strerror_r(errnum, buf, buflen);
    return buf;
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
        // TODO: use mmap64 in this case
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

/**
 * cpuid routines: for libchelper.a
 * The cpuid.c file is removed from libchelper in graal to
 * prevent multiple definitions.
 * The version of libchelper.a does not contain the below
 * defitions; they were then added back again to cpuid.c in graal to rebuild
 * other native images like gu correctly.
 *
 */

extern "C"
{
    char *SVM_FindJavaTZmd(const char *tzmappings, int length)
    {
        GRAAL_SGX_INFO();
        char *ret = "dummy_tz";
        return ret;
    }
}

pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
} // out
pid_t vfork(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
pid_t fork(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int statvfs64(const char *path, struct statvfs *buf)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
} // out
int execve(const char *pathname, char *const argv[], char *const envp[])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int execvp(const char *file, char *const argv[])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
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
    //printDlinfo(info, ret);

    return ret;
}

int printDlinfo(void *info, int ret)
{
    Dl_info *dlinfo = (Dl_info *)info;
    printf("------ DL information. Ret value: %d --------\n", ret);
    if (dlinfo->dli_fname != nullptr)
    {
        printf("pathname of so --> dli_fname: %s\n", dlinfo->dli_fname);
    }
    if (dlinfo->dli_sname)
    {
        printf("symbol name at addr --> dli_sname: %s\n", dlinfo->dli_sname);
    }

    printf("------ End DL info -------\n");
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
