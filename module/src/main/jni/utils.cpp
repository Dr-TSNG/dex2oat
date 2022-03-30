#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"
#include "log.h"

ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags) {
    int sent = sendmsg(sockfd, msg, flags);
    if (sent < 0) {
        PLOGE("sendmsg");
    }
    return sent;
}

ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags) {
    int rec = recvmsg(sockfd, msg, flags);
    if (rec < 0) {
        PLOGE("recvmsg");
    }
    return rec;
}

// Read exact same size as count
ssize_t xxread(int fd, void *buf, size_t count) {
    size_t read_sz = 0;
    ssize_t ret;
    do {
        ret = read(fd, (std::byte *) buf + read_sz, count - read_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            PLOGE("read");
            return ret;
        }
        read_sz += ret;
    } while (read_sz != count && ret != 0);
    if (read_sz != count) {
        PLOGE("read (%zu != %zu)", count, read_sz);
    }
    return read_sz;
}

// Write exact same size as count
ssize_t xwrite(int fd, const void *buf, size_t count) {
    size_t write_sz = 0;
    ssize_t ret;
    do {
        ret = write(fd, (std::byte *) buf + write_sz, count - write_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            PLOGE("write");
            return ret;
        }
        write_sz += ret;
    } while (write_sz != count && ret != 0);
    if (write_sz != count) {
        PLOGE("write (%zu != %zu)", count, write_sz);
    }
    return write_sz;
}

int
send_fds(int sockfd, void *cmsgbuf, size_t bufsz, const int *fds, int cnt, void *data,
         size_t datasz) {
    iovec iov = {
            .iov_base = data ? data : &cnt,
            .iov_len  = datasz,
    };
    msghdr msg = {
            .msg_iov        = &iov,
            .msg_iovlen     = 1,
    };

    if (cnt) {
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = bufsz;
        cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * cnt);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        memcpy(CMSG_DATA(cmsg), fds, sizeof(int) * cnt);
    }

    return xsendmsg(sockfd, &msg, 0);
}

int send_fd(int sockfd, int fd) {
    if (fd < 0) {
        return send_fds(sockfd, nullptr, 0, nullptr, 0);
    }
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    return send_fds(sockfd, cmsgbuf, sizeof(cmsgbuf), &fd, 1);
}

void *recv_fds(int sockfd, char *cmsgbuf, size_t bufsz, int cnt, void *data, size_t datasz) {
    iovec iov = {
            .iov_base = data ? data : &datasz,
            .iov_len  = datasz,
    };
    msghdr msg = {
            .msg_iov        = &iov,
            .msg_iovlen     = 1,
            .msg_control    = cmsgbuf,
            .msg_controllen = bufsz
    };

    xrecvmsg(sockfd, &msg, MSG_WAITALL);
    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    if (msg.msg_controllen != bufsz ||
        cmsg == nullptr ||
        cmsg->cmsg_len != CMSG_LEN(sizeof(int) * cnt) ||
        cmsg->cmsg_level != SOL_SOCKET ||
        cmsg->cmsg_type != SCM_RIGHTS) {
        return nullptr;
    }

    return CMSG_DATA(cmsg);
}

int recv_fd(int sockfd) {
    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    void *data = recv_fds(sockfd, cmsgbuf, sizeof(cmsgbuf), 1);
    if (data == nullptr)
        return -1;

    int result;
    memcpy(&result, data, sizeof(int));
    return result;
}

int read_int(int fd) {
    int val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return val;
}

void write_int(int fd, int val) {
    if (fd < 0) return;
    xwrite(fd, &val, sizeof(val));
}
