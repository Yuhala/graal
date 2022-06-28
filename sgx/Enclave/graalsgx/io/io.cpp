
/*
 * Created on Tue Jul 21 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

/**
 * Quick note on TODOs: most of the routines should be simple to reimplement with ocalls.
 * A few may require special attention. For example routines with struct or other complex
 * types as return or param types.
 */

#include "checks.h"        //for pointer checks
#include "../../Enclave.h" //for printf

SGX_FILE stdin = SGX_STDIN;
SGX_FILE stdout = SGX_STDOUT;
SGX_FILE stderr = SGX_STDERR;

extern char **environ;

void sgx_exit()
{
    GRAAL_SGX_INFO();
    exit(1);
}
int fsync(int fd)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fsync(&ret, fd);
    return ret;
}

int dup2(int oldfd, int newfd)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_dup2(&ret, oldfd, newfd);
    return ret;
}

int open(const char *path, int oflag, ...)
{
    GRAAL_SGX_INFO();
    va_list ap;
    va_start(ap, oflag);
    int arg = va_arg(ap, int);
    va_end(ap);

    int ret;
    int sgx_ret = ocall_open(&ret, path, oflag, arg);

    if (sgx_ret != SGX_SUCCESS)
    {
        printf("Error in open OCALL\n");
        sgx_exit();
    }

    return ret;
}
int open64(const char *path, int oflag, ...)
{
    GRAAL_SGX_INFO();
    va_list ap;
    va_start(ap, oflag);
    int arg = va_arg(ap, int);
    va_end(ap);

    int ret;
    int sgx_ret = ocall_open64(&ret, path, oflag, arg);

    if (sgx_ret != SGX_SUCCESS)
    {
        printf("Error in open64 OCALL\n");
        sgx_exit();
    }

    return ret;
}
int close(int fd)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_xclose(&ret, fd);
    return ret;
}

SGX_FILE fopen(const char *pathname, const char *mode)
{
    GRAAL_SGX_INFO();
    SGX_FILE f = 0;
    ocall_fopen(&f, pathname, mode);
    return f;
}
SGX_FILE fdopen(int fd, const char *mode)
{
    GRAAL_SGX_INFO();
    SGX_FILE f = 0;
    ocall_fdopen(&f, fd, mode);
    return f;
}
/* SGX_FILE stderr()
{
    GRAAL_SGX_INFO();
    SGX_FILE f = 0;
    ocall_stderr(&f);
    return f;
} */

int fclose(SGX_FILE stream)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fclose(&ret, stream);
    return ret;
}
size_t fwrite(const void *ptr, size_t size, size_t nmemb, SGX_FILE f)
{
    GRAAL_SGX_INFO();
    size_t ret = 0;
    ocall_fwrite(&ret, ptr, size, nmemb, f);
    return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, SGX_FILE f)
{
    GRAAL_SGX_INFO();
    size_t ret = 0;
    ocall_fread(&ret, ptr, size, nmemb, f);

    return ret;
}

int puts(const char *str)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_puts(&ret, str);
    return ret;
}
int fscanf(SGX_FILE stream, const char *fmt, ...)
{ // undefined behaviour at runtime
    GRAAL_SGX_INFO();
    int ret = 0;
    // obtain additional arguments
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_fscanf(&ret, stream, buf);
    return ret;
}
int fprintf(SGX_FILE stream, const char *fmt, ...)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // obtain additional arguments
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    // int len = (int)strnlen(buf, BUFSIZ - 1) + 1;
    ocall_fprintf(&ret, stream, buf);
    return ret;
}
char *fgets(char *str, int n, SGX_FILE stream)
{
    GRAAL_SGX_INFO();
    // printf("Fget str: %s\n", str);
    ocall_fgets(str, n, stream);
    return str;
}

ssize_t read(int fd, void *buf, size_t count)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_read(&ret, fd, buf, count);
    // printf("read fd: %d\n", fd);
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    // printf(">>>>>>>>>>>>> write fd: %d\n", fd);
    ocall_write(&ret, fd, buf, count);
    return ret;
}
int sprintf(char *str, const char *fmt, ...)
{
    // GRAAL_SGX_INFO();
    // this should work but may need revision
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    int size = (int)strnlen(buf, BUFSIZ - 1) + 1 + BUFSIZ;
    return snprintf(str, size_t(size), fmt);
}
int vfprintf(SGX_FILE *stream, const char *format, va_list ap)
{
    GRAAL_SGX_INFO();
    // TODO
    return 0;
}

char *strcpy(char *dest, const char *src)
{
    GRAAL_SGX_INFO();
    return strncpy(dest, src, strnlen(src, BUFSIZ - 1) + 1);
}

char *strcat(char *dest, const char *src)
{
    GRAAL_SGX_INFO();
    return strncat(dest, src, strnlen(src, BUFSIZ - 1) + 1);
}

void *opendir(const char *name)
{
    printf("Opendir name: %s\n", name);
    GRAAL_SGX_INFO();
    void *ret;
    ocall_opendir(&ret, name);
    return ret;
}

// void *fdopendir(int fd);
int closedir(void *dirp)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret;
    ocall_closedir(&ret, dirp);
    return ret;
}
// struct dirent *readdir(void *dirp);
int readdir64_r(void *dirp, struct dirent *entry, struct dirent **result)
{
    GRAAL_SGX_INFO();
    // TODO
    int ret;
    ocall_readdir64_r(&ret, dirp, entry, result);
    return ret;
}
// int remove(const char *pathname);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_readlink(&ret, pathname, buf, bufsiz);
    return ret;
}
long pathconf(const char *path, int name)
{
    GRAAL_SGX_INFO();
    long ret;
    ocall_pathconf(&ret, path, name);
    return ret;
}
int __xstat(int ver, const char *path, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_xstat(&ret, ver, path, stat_buf);
    return ret;
}
int __lxstat(int ver, const char *path, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_lxstat(&ret, ver, path, stat_buf);
    return ret;
}
int __fxstat(int ver, int fd, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_fxstat(&ret, ver, fd, stat_buf);
    return ret;
}

int fstat64(int fd, struct stat *buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_fstat64(&ret, fd, buf);
    return 0;
}
int __fxstat64(int ver, int fd, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_fxstat64(&ret, ver, fd, stat_buf);
    return ret;
}

char *getenv(const char *name)
{
    GRAAL_SGX_INFO();

    char *retval;
    sgx_status_t status = ocall_getenv(&retval, name);
    // CHECK_STATUS(status);
    // printf(">>>>>>>>>>>>>>>>>>> get env: %s result: %s\n", name, retval);
    return retval;
}

ulong crc32(ulong crc, const Byte *buf, uint len)
{
    GRAAL_SGX_INFO();
    // TODO
}

int mkdir(const char *pathname, mode_t mode)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_mkdir(&ret, pathname, mode);
    return ret;
}
int truncate(const char *path, off_t length)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_truncate(&ret, path, length);
    return ret;
}
int ftruncate64(int fd, off_t length)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_ftruncate64(&ret, fd, length);
    return ret;
}
void *mmap64(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
    /**
     * mmap64 is mostly used to map [large]files in the application VAS.
     * To our knowledge so far, graal uses mmap to allocate heap memory for apps; we allocate that
     * in sgx reserved memory. mmap64 does not use sgx reserve memory for now. fd != -1 here.
     */

    GRAAL_SGX_INFO();
    // printf("In mmap 64: fd = %d\n", fd);
    // return mmap(addr, len, prot, flags, fd, off);
    void *ret = nullptr;
    ocall_mmap64(&ret, addr, len, prot, flags, fd, off);
    return ret;
}
ssize_t pwrite64(int fd, const void *buf, size_t nbyte, off_t offset)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_pwrite64(&ret, fd, buf, nbyte, offset);
    return ret;
}
int fdatasync(int fd)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fdatasync(&ret, fd);
    return ret;
}
int rename(const char *oldpath, const char *newpath)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_rename(&ret, oldpath, newpath);
    return ret;
}
int unlink(const char *pathname)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_unlink(&ret, pathname);
    return ret;
}
int rmdir(const char *pathname)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_rmdir(&ret, pathname);
    return ret;
}
clock_t times(struct tms *buf)
{
    GRAAL_SGX_INFO();
    clock_t ret = 0;
    ocall_times(&ret);
    return ret;
}
int utimes(const char *filename, const struct timeval times[2])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int chown(const char *pathname, uid_t owner, gid_t group)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_chown(&ret, pathname, owner, group);
    return ret;
}
int fchown(int fd, uid_t owner, gid_t group)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fchown(&ret, fd, owner, group);
    return ret;
}
int lchown(const char *pathname, uid_t owner, gid_t group)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_lchown(&ret, pathname, owner, group);
    return ret;
}
int chmod(const char *pathname, mode_t mode)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_chmod(&ret, pathname, mode);
    return ret;
}
int fchmod(int fd, mode_t mode)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fchmod(&ret, fd, mode);
    return ret;
}
int __lxstat64(int ver, const char *path, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_lxstat64(&ret, ver, path, stat_buf);
    return ret;
}
int __xmknod(int vers, const char *path, mode_t mode, dev_t *dev)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_xmknod(&ret, vers, path, mode, dev);
    return ret;
}
int symlink(const char *target, const char *linkpath)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_symlink(&ret, target, linkpath);
    return ret;
}
int deflateEnd(z_streamp stream)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_deflateEnd(&ret, stream);
    return ret;
}
int deflateParams(z_streamp stream, int level, int strategy)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_deflateParams(&ret, stream, level, strategy);
    return ret;
}
int deflate(z_streamp stream, int flush)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_deflate(&ret, stream, flush);
    return ret;
}
int deflateInit2_(z_streamp stream, int level, int method, int windowBits, int memLevel, int strategy)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_deflateInit2(&ret, stream, level, method, windowBits, memLevel, strategy);
    return ret;
}
int inflateReset(z_streamp stream)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_inflateReset(&ret, stream);
    return ret;
}
ssize_t sendfile64(int out_fd, int in_fd, off_t *offset, size_t count)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_sendfile64(&ret, out_fd, in_fd, offset, count);
    return ret;
}
ulong adler32(ulong adler, const Bytef *buf, size_t len)
{
    GRAAL_SGX_INFO();
    ulong ret = 0;
    ocall_adler32(&ret, adler, buf, len);
    return ret;
}

void set_environ(void **env)
{
    GRAAL_SGX_INFO();
    environ = (char **)env;
}

// Added for graal 21.0
int __libc_current_sigrtmax(void)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
off_t lseek(int fd, off_t offset, int whence)
{
    GRAAL_SGX_INFO();
    off_t ret = 0;
    ocall_lseek(&ret, fd, offset, whence);
    return ret;
}
struct dirent *readdir(DIR *dirp)
{
    GRAAL_SGX_INFO();
    // TODO
    return nullptr;
}
struct dirent *readdir64(DIR *dirp)
{
    GRAAL_SGX_INFO();

    // TODO
    return nullptr;
}
int ioctl(int fd, unsigned long request, ...)
{
    GRAAL_SGX_INFO();
    va_list ap;
    va_start(ap, request);
    int arg = va_arg(ap, int);
    va_end(ap);

    int ret;
    int sgx_ret = ocall_ioctl(&ret, fd, request, arg);

    if (sgx_ret != SGX_SUCCESS)
    {
        printf("Error in fcntl OCALL\n");
        sgx_exit();
    }

    return ret;
}
off64_t lseek64(int fd, off64_t offset, int whence)
{
    GRAAL_SGX_INFO();
    off64_t ret = 0;
    ocall_lseek64(&ret, fd, offset, whence);
    return ret;
}
int fflush(SGX_FILE *stream)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_fflush(&ret, stream);
    return ret;
}

const char *gai_strerror(int ecode)
{
    GRAAL_SGX_INFO();
    const char *ret = "gai_strerror";
    // TODO
    return ret;
}
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_pread(&ret, fd, buf, count, offset);
    return ret;
}
ssize_t pread64(int fd, void *buf, size_t count, off64_t offset)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_pread64(&ret, fd, buf, count, offset);
    return 0;
}
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_pwrite(&ret, fd, buf, count, offset);
    return 0;
}
int fcntl(int fd, int cmd, ... /* arg */)
{
    GRAAL_SGX_INFO();
    va_list ap;
    va_start(ap, cmd);
    int arg = va_arg(ap, int);
    va_end(ap);

    int ret;
    int sgx_ret = ocall_fcntl(&ret, fd, cmd, arg);

    if (sgx_ret != SGX_SUCCESS)
    {
        printf("Error in fcntl OCALL\n");
        sgx_exit();
    }

    return ret;
}
int fstatvfs64(int fd, struct statvfs *buf)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}

int __xstat64(int ver, const char *path, struct stat *stat_buf)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_xstat64(&ret, ver, path, stat_buf);
    return ret;
}

int pthread_kill(pthread_t thread, int sig)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int inflateInit2_(z_streamp strm, int windowBits, char *version, int stream_size)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int inflate(z_streamp stream, int flush)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int inflateEnd(z_streamp stream)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}

int dup(int oldfd)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}

int access(const char *pathname, int mode)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    // TODO
    return ret;
}
int chdir(const char *path)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_chdir(&ret, path);
    return ret;
}
int fileno(SGX_FILE *stream)
{
    GRAAL_SGX_INFO();
    if (stream == nullptr)
    {
        printf("fileno stream is null\n");
        return -1;
    }
    int ret;
    ocall_fileno(&ret, stream);
    return ret;
}
int isatty(int fd)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_isatty(&ret, fd);
    return ret;
}
mode_t umask(mode_t mask)
{
    GRAAL_SGX_INFO();
    mode_t ret;
    ocall_umask(&ret, mask);
    return ret;
}