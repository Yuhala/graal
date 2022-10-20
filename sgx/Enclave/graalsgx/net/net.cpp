/*
 * Created on Wed Jul 22 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */
#include "../../checks.h" //for pointer checks
#include "../../Enclave.h"
#include "inet_pton.h"

long timezone = 0; // TODO

int socket(int domain, int type, int protocol)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_socket(&ret, domain, type, protocol);
    return ret;
}
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getsockname(&ret, sockfd, addr, addrlen);
    return ret;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    GRAAL_SGX_INFO();
    int ret;
    ocall_clock_gettime(&ret, clk_id, tp, sizeof(struct timespec));
    return ret;
}

int gettimeofday(struct timeval *tv, void *tz)
{
    GRAAL_SGX_INFO();
    int ret;
    sgx_status_t sgx_ret;
    if ((sgx_ret = ocall_gettimeofday(&ret, tv, sizeof(struct timeval))) != SGX_SUCCESS)
    {
        printf("OCALL FAILED!, Error code = %d\n", sgx_ret);
        ocall_exit(EXIT_FAILURE);
    }
    // ocall_gettimeofday(&ret, tv, tz);
    return ret;
}

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getaddrinfo(&ret, node, service, hints, res);
    return ret;
}

void freeaddrinfo(struct addrinfo *res)
{
    GRAAL_SGX_INFO();
    ocall_freeaddrinfo(res);
}

pid_t getpid(void)
{
    GRAAL_SGX_INFO();
    pid_t ret = 0;
    ocall_getpid(&ret);
    return ret;
}
int remove(const char *pathname)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_remove(&ret, pathname);
    return ret;
}

int shutdown(int sockfd, int how)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_shutdown(&ret, sockfd, how);
    return ret;
}
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getsockopt(&ret, sockfd, level, optname, optval, optlen);
    return ret;
}
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_setsockopt(&ret, sockfd, level, optname, optval, optlen);
    return ret;
}

int socketpair(int domain, int type, int protocol, int sv[2])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_socketpair(&ret, domain, type, protocol, sv);
    return ret;
}

int bind(int sockfd, const struct sockaddr *addr,
         socklen_t addrlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_bind(&ret, sockfd, addr, addrlen);
    return ret;
}

int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_epoll_wait(&ret, epfd, events, maxevents, timeout);
    return ret;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_epoll_ctl(&ret, epfd, op, fd, event);
    return ret;
}
int epoll_create(int size)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_epoll_create(&ret, size);
    return ret;
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ocall_readv(&ret, fd, iov, iovcnt);
    return ret;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    GRAAL_SGX_INFO();
    ssize_t ret = 0;
    ret = ocall_writev(&ret, fd, iov, iovcnt);
    return ret;
}

int pipe(int pipefd[2])
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ret = ocall_pipe(&ret, pipefd);
    return ret;
}
int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_connect(&ret, sockfd, addr, addrlen);
    return ret;
}

int listen(int sockfd, int backlog)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ret = ocall_listen(&ret, sockfd, backlog);
    return ret;
}
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_accept(&ret, sockfd, addr, addrlen);
    return ret;
}
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_poll(&ret, fds, nfds, timeout);
    return ret;
}
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_getnameinfo(&ret, addr, addrlen, host, hostlen, serv, servlen, flags);
    return ret;
}
int gethostname(char *name, size_t len)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_gethostname(&ret, name, len);
    return ret;
}
int sethostname(const char *name, size_t len)
{
    GRAAL_SGX_INFO();
    int ret = 0;
    ocall_sethostname(&ret, name, len);
    return ret;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_recv(&ret, sockfd, buf, len, flags);
    return ret;
}
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_send(&ret, sockfd, buf, len, flags);
    return ret;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_sendto(&ret, sockfd, buf, len, flags, dest_addr, addrlen);
    return ret;
}

ssize_t sendmsg(int sockfd, struct msghdr *msg, int flags)
{
    GRAAL_SGX_INFO();
    ssize_t ret;

    ocall_sendmsg(&ret, sockfd, msg, flags);

    return ret;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_recvfrom(&ret, sockfd, buf, len, flags, src_addr, addrlen);
    return ret;
}
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    GRAAL_SGX_INFO();
    ssize_t ret;
    ocall_recvmsg(&ret, sockfd, msg, flags);
    return ret;
}
