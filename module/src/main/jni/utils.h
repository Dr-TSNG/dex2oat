#pragma once

#include <cstdlib>

#if defined(__LP64__)
#define LP_SELECT(lp32, lp64) lp64
#else
#define LP_SELECT(lp32, lp64) lp32
#endif

ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags);

ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags);

// Read exact same size as count
ssize_t xxread(int fd, void *buf, size_t count);

// Write exact same size as count
ssize_t xwrite(int fd, const void *buf, size_t count);

int
send_fds(int sockfd, void *cmsgbuf, size_t bufsz, const int *fds, int cnt, void *data = nullptr,
         size_t datasz = 1);

int send_fd(int sockfd, int fd);

void *recv_fds(int sockfd, char *cmsgbuf, size_t bufsz, int cnt, void *data = nullptr,
               size_t datasz = 1);

int recv_fd(int sockfd);

int read_int(int fd);

void write_int(int fd, int val);
