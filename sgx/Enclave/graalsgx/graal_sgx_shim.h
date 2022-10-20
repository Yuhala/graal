/*
 * Created on Wed Jul 15 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

#ifndef GRAAL_SGX_SHIM_H
#define GRAAL_SGX_SHIM_H

//#define __USE_LARGEFILE64 //for stat64

// used by some reimplementations. For testing purposes --PYuhala
#define GRAAL_SGX_STACK_SIZE 0x200000
#define GRAAL_SGX_PAGESIZE 4096
#define GRAAL_SGX_GUARDSIZE GRAAL_SGX_PAGESIZE
#define GRAAL_SCHED_POLICY 0
#define COND_NWAITERS_SHIFT 1
#define CLOCK_MONOTONIC 0
#define CLOCK_REALTIME 1
#define NUM_MAPS 100000
#define Pause() __asm__ __volatile__("pause" \
                                     :       \
                                     :       \
                                     : "memory")

#define __USE_LARGEFILE64
// sys
#include <sgx/sys/types.h>
#include <sgx/sys/stat.h>
#include <sgx/pwd.h>
#include <sgx/sys/utsname.h>
#include <sgx/sys/resource.h>
#include <sgx/linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <sgx/signal.h>
#include <unistd.h>
#include <sgx/netdb.h>
#include <struct/sgx_fcntl_struct.h>
#include <sgx/sys/wait.h>
//#include <sgx/sys/statvfs.h>

// io
#include <stdio.h>

#include <sgx/dirent.h>
//#include <struct/sgx_stdio_struct.h>
//#include <struct/sgx_sysstat_struct.h>

// net
#include <sgx/sys/socket.h>
#include <sgx/arpa/inet.h>
#include <sgx/sys/epoll.h>
#include <sgx/sys/uio.h>
#include <sgx/sys/poll.h>
#include <sgx/sys/epoll.h>

#include <zlib/zlib.h>

// threads
//#include <pthread.h>
#include <struct/sgx_pthread_struct.h>
#include <sgx_thread.h>
//#include <sgx_pthread.h>

#include "thread/stack.h"

// TYPES
typedef unsigned char Byte;
typedef unsigned char Bytef;
typedef long off64_t;
typedef size_t z_size_t;
typedef void DIR;

struct mntent
{
    char *mnt_fsname; /* name of mounted filesystem */
    char *mnt_dir;    /* filesystem path prefix */
    char *mnt_type;   /* mount type (see mntent.h) */
    char *mnt_opts;   /* mount options (see mntent.h) */
    int mnt_freq;     /* dump frequency in days */
    int mnt_passno;   /* pass number on parallel fsck */
};

// Dynamic loading

typedef struct
{
    const char *dli_fname; /* Pathname of shared object that
                              contains address */
    void *dli_fbase;       /* Base address at which shared
                              object is loaded */
    const char *dli_sname; /* Name of symbol whose definition
                              overlaps addr */
    void *dli_saddr;       /* Exact address of symbol named
                              in dli_sname */
} Dl_info;

/* Binary semaphore */
typedef struct sgx_bsem_t
{
    sgx_thread_mutex_t mutex;
    sgx_thread_cond_t cond;
    int v;
} sgx_bsem_t;

// typedef int Z_STREAMP;
struct statvfs
{
    int todo;
};

// extern char **environ;
//  This prevents "name mangling" by g++ ---> PYuhala
#if defined(__cplusplus)
extern "C"
{
#endif

    // custom routines
    void sgx_exit();
    // sys
    void *dlsym(void *handle, const char *symbol);
    void *dlopen(const char *filename, int flag);
    int dladdr(const void *addr, void *info);
    int kill(int pid, int sig);
    void *dlmopen(long lmid, const char *filename, int flags);
    int dlclose(void *handle);
    int dlinfo(void *handle, int request, void *info);
    char *dlerror();

    int getgrgid_r(unsigned int gid, void *grp, char *buf, size_t buflen, void **result);
    int getpwuid_r(unsigned int uid, void *pwd, char *buf, size_t buflen, void **result);

    long sysconf(int name);
    int fstat64(int fd, struct stat *buf);
    int __fxstat64(int ver, int fildes, struct stat *stat_buf);
    int __xstat64(int ver, const char *path, struct stat *stat_buf);

    int __xstat(int ver, const char *path, struct stat *stat_buf);
    int __lxstat(int ver, const char *path, struct stat *stat_buf);
    int __fxstat(int ver, int fildes, struct stat *stat_buf);

    ulong crc32(ulong crc, const Byte *buf, uint len);
    uid_t getuid(void);

    // cpuid: for libchelper.a
    unsigned int get_cpuid_max(unsigned int ext, unsigned int *sig);
    int get_cpuid_count(unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
    int get_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);

    // mem management
    int munmap(void *addr, size_t length);
    void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

    // clock
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    int gettimeofday(struct timeval *tv, void *tz);

    char *getcwd(char *buf, size_t size);
    struct passwd *getpwuid(uid_t uid);
    void exit(int status);
    int getrlimit(int res, struct rlimit *rlim);
    int setrlimit(int resource, const struct rlimit *rlim);
    long syscall(long num, ...);
    int uname(struct utsname *buf);
    unsigned int sleep(unsigned int secs);
    int mprotect(void *addr, size_t len, int prot);
    char *realpath(const char *path, char *resolved_path);
    char *__xpg_strerror_r(int errnum, char *buf, size_t buflen);

    // SIGNAL ROUTINES
    int sigaction(int signum, struct sigaction *act, struct sigaction *oldact);
    int sigfillset(sigset_t *set);
    int sigaddset(sigset_t *set, int signum);
    int sigdelset(sigset_t *set, int signo);
    int sigemptyset(sigset_t *set);
    int sigismember(const sigset_t *set, int signo);
    int sigsuspend(const sigset_t *set);
    int sigpending(sigset_t *set);
    int sigwait(const sigset_t *set, int *sig);
    int raise(int signal);

    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
    sighandler_t signal(int signum, sighandler_t handler);
    void sig_handler(int param);
    int nanosleep(const struct timespec *__requested_time, struct timespec *__remaining);

    // IO routines
    int fsync(int fd);
    int dup2(int oldfd, int newfd);
    int open(const char *path, int oflag, ...);
    int open64(const char *path, int oflag, ...);
    int close(int fd);
    SGX_FILE fopen(const char *pathname, const char *mode);
    SGX_FILE fdopen(int fd, const char *mode);
    int mkostemp(char *tmplate, int flags);

    // SGX_FILE stderr();
    int fclose(SGX_FILE stream);
    int fscanf(SGX_FILE stream, const char *format, ...);
    int fprintf(SGX_FILE stream, const char *fmt, ...);
    int vfprintf(SGX_FILE *stream, const char *format, va_list ap);
    char *fgets(char *str, int n, SGX_FILE stream);
    int puts(const char *str);

    // IO
    int mkdir(const char *pathname, mode_t mode);
    int truncate(const char *path, off_t length);
    int ftruncate64(int fd, off_t length);
    int ftruncate(int fd, off_t length);
    void *mmap64(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
    ssize_t pwrite64(int fildes, const void *buf, size_t nbyte, off_t offset);
    int fdatasync(int fd);
    int rename(const char *oldpath, const char *newpath);
    int unlink(const char *pathname);
    int rmdir(const char *pathname);
    clock_t times(struct tms *buf);
    int utimes(const char *filename, const struct timeval times[2]);

    int chown(const char *pathname, uid_t owner, gid_t group);
    int fchown(int fd, uid_t owner, gid_t group);
    int lchown(const char *pathname, uid_t owner, gid_t group);
    int chmod(const char *pathname, mode_t mode);
    int fchmod(int fd, mode_t mode);
    int __lxstat64(int ver, const char *path, struct stat *stat_buf);
    int __xmknod(int vers, const char *path, mode_t mode, dev_t *dev);
    int symlink(const char *target, const char *linkpath);

    // ZLIB ROUTINES

    int inflateSetDictionary(Z_STREAMP stream, const Bytef *dictionary, uInt dictlen);
    int inflateInit2_(Z_STREAMP strm, int windowBits, char *version, int stream_size);
    int inflate(Z_STREAMP stream, int flush);
    int inflateEnd(Z_STREAMP stream);
    int deflateReset(Z_STREAMP stream);

    int deflateEnd(Z_STREAMP stream);
    int deflateParams(Z_STREAMP stream, int level, int strategy);
    int deflate(Z_STREAMP stream, int flush);
    int deflateInit2_(Z_STREAMP stream, int level, int method, int windowBits, int memLevel, int strategy);
    int deflateSetDictionary(Z_STREAMP stream, const Bytef *dictionary, uInt dictlen);
    int inflateReset(Z_STREAMP stream);

    SGX_FILE setmntent(const char *filename, const char *type);
    struct mntent *getmntent(SGX_FILE fp);
    struct mntent *getmntent_r(SGX_FILE streamp, struct mntent *mntbuf, char *buf, int buflen);
    int addmntent(SGX_FILE fp, const struct mntent *mnt);
    int endmntent(SGX_FILE fp);
    char *hasmntopt(const struct mntent *mnt, const char *opt);

    int statfs(const char *path, struct statfs *buf);
    int fstatfs(int fd, struct statfs *buf);
    char *strtok_r(char *str, const char *delim, char **saveptr);
    char *__strtok_r(char *str, const char *delim, char **saveptr);
    int getgroups(int size, gid_t *list);

    //----------------------------------------------------------

    void *opendir(const char *name);
    // void *fdopendir(int fd);
    int closedir(void *dirp);
    // struct dirent *readdir(void *dirp);
    int readdir64_r(void *dirp, struct dirent *entry, struct dirent **result);
    int remove(const char *pathname);
    ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);
    long pathconf(const char *path, int name);
    char *getenv(const char *name);

    size_t fwrite(const void *ptr, size_t size, size_t nmemb, SGX_FILE file);
    size_t fread(void *ptr, size_t size, size_t nmemb, SGX_FILE file);
    ssize_t read(int fd, void *buf, size_t count);
    ssize_t write(int fd, const void *buf, size_t count);
    int sprintf(char *str, const char *format, ...);

    // STRING routines

    char *strcpy(char *dest, const char *src);
    char *strcat(char *dest, const char *src);
    char *stpcpy(char *dest, const char *src);

    // LOCALE routines
    int32_t **__ctype_toupper_loc(void);
    const unsigned short **__ctype_b_loc(void);

    // NETWORKING routines

    int socket(int domain, int type, int protocol);
    int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int inet_pton(int af, const char *src, void *dst);

    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
    ssize_t sendmsg(int sockfd, struct msghdr *msg, int flags);

    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

    // THREADS
    int pthread_create(pthread_t *thread, GRAAL_SGX_PTHREAD_ATTR attr, void *(*start_routine)(void *), void *arg);
    pthread_t pthread_self(void);
    int pthread_join(pthread_t thread, void **retval); // better join outside after pthread_create :-)

    int pthread_attr_getguardsize(GRAAL_SGX_PTHREAD_ATTR attr, size_t *guardsize);
    int pthread_attr_destroy(GRAAL_SGX_PTHREAD_ATTR attr);
    int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize);
    int pthread_getattr_np(pthread_t thread, GRAAL_SGX_PTHREAD_ATTR attr);
    int pthread_attr_setdetachstate(GRAAL_SGX_PTHREAD_ATTR attr, int detachstate);
    int pthread_attr_init(GRAAL_SGX_PTHREAD_ATTR attr);
    int pthread_setname_np(pthread_t thread, const char *name);
    int pthread_getname_np(pthread_t thread, char *name, size_t len);
    int pthread_attr_setstacksize(GRAAL_SGX_PTHREAD_ATTR attr, size_t stacksize);

    int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);
    int pthread_condattr_init(pthread_condattr_t *attr);
    int pthread_cond_timedwait(pthread_cond_t *__restrict__ cond, pthread_mutex_t *__restrict__ mutex, const struct timespec *__restrict__ abstime);
    int pthread_cond_broadcast(pthread_cond_t *cond);
    int pthread_cond_wait(pthread_cond_t *__restrict__ cond, pthread_mutex_t *__restrict__ mutex);
    int pthread_cond_init(pthread_cond_t *__restrict__ cond, const pthread_condattr_t *__restrict__ attr);

    int pthread_mutex_init(pthread_mutex_t *__restrict__ mutex, const pthread_mutexattr_t *__restrict__ attr);
    int pthread_mutex_lock(pthread_mutex_t *mutex);
    int pthread_mutex_trylock(pthread_mutex_t *mutex);
    int pthread_mutex_unlock(pthread_mutex_t *mutex);

    int sched_yield(void);

    int sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask);
    int sched_getaffinity(pid_t pid, size_t cpusetsize, void *mask);
    int __sched_cpucount(size_t setsize, const void *setp);

    // job
    bool get_pthreadid_from_sgxtheadid(sgx_thread_t sgx_id, pthread_t *pt);
    void ecall_execute_job(pthread_t pthread_id, unsigned long int job_id);
    void *graal_job(void *arg); // graal_sgx_thread function

    // SEMAPHORE routines
    void sem_init(struct sgx_bsem_t *bsem_p, int value);
    void sem_reset(struct sgx_bsem_t *bsem_p);
    void sem_post(struct sgx_bsem_t *bsem_p);
    void sem_post_all(struct sgx_bsem_t *bsem_p);
    void sem_wait(struct sgx_bsem_t *bsem_p);
    int sem_destroy(struct sgx_bsem_t *bsem_p);

    // Added for graal 21.0
    int __libc_current_sigrtmax(void);
    off_t lseek(int fd, off_t offset, int whence);
    struct dirent *readdir(DIR *dirp);
    struct dirent *readdir64(DIR *dirp);
    int ioctl(int fd, unsigned long request, ...);
    off64_t lseek64(int fd, off64_t offset, int whence);
    int fflush(SGX_FILE *stream);

    int getaddrinfo(const char *node, const char *service,
                    const struct addrinfo *hints,
                    struct addrinfo **res);

    void freeaddrinfo(struct addrinfo *res);

    const char *gai_strerror(int ecode);
    ssize_t pread(int fd, void *buf, size_t count, off_t offset);
    ssize_t pread64(int fd, void *buf, size_t count, off64_t offset);
    ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
    int fcntl(int fd, int cmd, ... /* arg */);
    int fstatvfs64(int fd, struct statvfs *buf);
    int pthread_kill(pthread_t thread, int sig);
    int dup(int oldfd);
    int access(const char *pathname, int mode);
    int getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
                    char *host, socklen_t hostlen,
                    char *serv, socklen_t servlen, int flags);
    int gethostname(char *name, size_t len);
    int sethostname(const char *name, size_t len);

    // Added for netty
    pid_t getpid(void);
    int remove(const char *pathname);
    int shutdown(int sockfd, int how);
    int getsockopt(int sockfd, int level, int optname,
                   void *optval, socklen_t *optlen);
    int setsockopt(int sockfd, int level, int optname,
                   const void *optval, socklen_t optlen);

    int socketpair(int domain, int type, int protocol, int sv[2]);
    int bind(int sockfd, const struct sockaddr *addr,
             socklen_t addrlen);

    int epoll_wait(int epfd, struct epoll_event *events,
                   int maxevents, int timeout);

    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
    ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
    int pipe(int pipefd[2]);
    int connect(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);

    int listen(int sockfd, int backlog);
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    int epoll_create(int size);

    // Added for halodb
    void set_environ(void **env);
    ssize_t sendfile64(int out_fd, int in_fd, off_t *offset, size_t count);
    ulong adler32(ulong adler, const Bytef *buf, size_t len);

    // Added for palDB
    pid_t waitpid(pid_t pid, int *wstatus, int options); // out
    pid_t vfork(void);
    pid_t fork(void);
    int statvfs64(const char *path, struct statvfs *buf); // out
    int execve(const char *pathname, char *const argv[], char *const envp[]);
    int execvp(const char *file, char *const argv[]);
    int chdir(const char *path);
    void _exit(int status);

    // Added for quickcached
    int fileno(SGX_FILE *stream); // in
    int isatty(int fd);
    mode_t umask(mode_t mask);

    // Added for graalvm build: June 1 2022 -- building polyglot applications
    int waitid(int idtype, int id, void *infop, int options);
    int posix_spawn(void *pid, void *path, void *file_actions, void *attrp, void **argv, void **envp);
    int sscanf(const char *str, const char *format);
    ssize_t __getdelim(char **lineptr, size_t *n, int delim, SGX_FILE *stream);

    // For libchelper redefinitions
    char *SVM_FindJavaTZmd(const char *tzmappings, int length);

#if defined(__cplusplus)
}
#endif

#endif /* GRAAL_SGX_SHIM_H */
