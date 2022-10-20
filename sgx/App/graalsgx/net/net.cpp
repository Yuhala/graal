/*
 * Created on Wed Jul 22 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include "ocall_logger.h"

#include "Enclave_u.h"

int ocall_socket(int domain, int type, int protocol)
{
    log_ocall(__func__);
    return socket(domain, type, protocol);
}

int ocall_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    log_ocall(__func__);
    return getsockname(sockfd, addr, addrlen);
}

int ocall_inet_pton(int af, const char *src, void *dst)
{
    log_ocall(__func__);
    return inet_pton(af, src, dst);
}

pid_t ocall_getpid(void)
{
    log_ocall(__func__);
    pid_t ret = 0;
    ret = getpid();
    return ret;
}
int ocall_remove(const char *pathname)
{
    log_ocall(__func__);
    int ret = 0;
    ret = remove(pathname);
    return ret;
}

int ocall_shutdown(int sockfd, int how)
{
    log_ocall(__func__);
    int ret = 0;
    ret = shutdown(sockfd, how);
    return ret;
}
int ocall_getsockopt(int sockfd, int level, int optname,
                     void *optval, socklen_t *optlen)
{
    log_ocall(__func__);
    int ret = 0;
    ret = getsockopt(sockfd, level, optname, optval, (socklen_t *)optlen);
    return ret;
}
int ocall_setsockopt(int sockfd, int level, int optname,
                     const void *optval, socklen_t optlen)
{
    log_ocall(__func__);
    int ret = 0;
    ret = setsockopt(sockfd, level, optname, optval, optlen);
    return ret;
}

int ocall_socketpair(int domain, int type, int protocol, int sv[2])
{
    log_ocall(__func__);
    int ret = 0;
    ret = socketpair(domain, type, protocol, sv);
    return ret;
}

int ocall_bind(int sockfd, const void *addr,
               socklen_t addrlen)
{
    log_ocall(__func__);
    int ret = 0;
    ret = bind(sockfd, (struct sockaddr *)addr, addrlen);
    return ret;
}

int ocall_epoll_wait(int epfd, struct epoll_event *events,
                     int maxevents, int timeout)
{
    log_ocall(__func__);
    int ret = 0;
    ret = epoll_wait(epfd, events, maxevents, timeout);
    return ret;
}

int ocall_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    log_ocall(__func__);
    int ret = 0;
    ret = epoll_ctl(epfd, op, fd, event);
    return ret;
}

//iov ->out
ssize_t ocall_readv(int fd, const struct iovec *iov, int iovcnt)
{
    log_ocall(__func__);
    ssize_t ret = 0;
    readv(fd, iov, iovcnt);
    return ret;
}
//iov ->in
ssize_t ocall_writev(int fd, const struct iovec *iov, int iovcnt)
{
    log_ocall(__func__);
    ssize_t ret = 0;
    ret = writev(fd, iov, iovcnt);
    return ret;
}
//out
int ocall_pipe(int pipefd[2])
{
    log_ocall(__func__);
    int ret = 0;
    ret = pipe(pipefd);
    return ret;
}
int ocall_connect(int sockfd, const void *addr,
                  socklen_t addrlen)
{
    log_ocall(__func__);
    int ret = 0;
    connect(sockfd, (struct sockaddr *)addr, addrlen);
    return ret;
}

int ocall_listen(int sockfd, int backlog)
{
    log_ocall(__func__);
    int ret = 0;
    ret = listen(sockfd, backlog);
    return ret;
}
int ocall_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    log_ocall(__func__);
    int ret = 0;
    ret = accept(sockfd, addr, addrlen);
    return ret;
}
int ocall_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    log_ocall(__func__);
    int ret = 0;
    poll(fds, nfds, timeout);
    return ret;
}
int ocall_epoll_create(int size)
{
    log_ocall(__func__);
    int ret = 0;
    epoll_create(size);
    return ret;
}
int ocall_getaddrinfo(const char *node, const char *service,
                      const struct addrinfo *hints,
                      struct addrinfo **res)
{
    log_ocall(__func__);
    return getaddrinfo(node, service, hints, res);
}

void ocall_freeaddrinfo(struct addrinfo *res)
{
    log_ocall(__func__);
    return freeaddrinfo(res);
}

int ocall_getnameinfo(const struct sockaddr *addr, socklen_t addrlen,
                      char *host, socklen_t hostlen,
                      char *serv, socklen_t servlen, int flags)
{
    log_ocall(__func__);
    return getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}
int ocall_gethostname(char *name, size_t len)
{
    log_ocall(__func__);
    return gethostname(name, len);
}
int ocall_sethostname(const char *name, size_t len)
{
    log_ocall(__func__);
    return sethostname(name, len);
}

int ocall_clock_gettime(clockid_t clk_id, void *tp, int ts_size)
{
    log_ocall(__func__);
    return clock_gettime(clk_id, (struct timespec *)tp);
}

int ocall_gettimeofday(void *tv, int tv_size)
{
    log_ocall(__func__);
    return gettimeofday((struct timeval *)tv, NULL);
}
ssize_t ocall_recv(int sockfd, void *buf, size_t len, int flags)
{
    log_ocall(__func__);
    return recv(sockfd, buf, len, flags);
}
ssize_t ocall_send(int sockfd, const void *buf, size_t len, int flags)
{
    log_ocall(__func__);
    return send(sockfd, buf, len, flags);
}

ssize_t ocall_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    log_ocall(__func__);
    return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t ocall_sendmsg(int sockfd, struct msghdr *msg, int flags)
{
    log_ocall(__func__);

    ssize_t ret = sendmsg(sockfd, msg, flags);
    //pyuhala:free message header here
    free(msg);
    return ret;
}

ssize_t ocall_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    log_ocall(__func__);
    return recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}
ssize_t ocall_recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    log_ocall(__func__);
    return recvmsg(sockfd, msg, flags);
}

