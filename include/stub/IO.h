#ifndef NETPP_STUB_IO_H
#define NETPP_STUB_IO_H

extern "C" {
#include <unistd.h>
}

struct iovec;
struct msghdr;

namespace netpp::internal::stub {
extern int close(int fd) noexcept;
extern ::ssize_t write(int fd, const void *buf, ::size_t count) noexcept;
extern ::ssize_t read(int fd, void *buf, ::size_t count) noexcept;
extern int pipe2(int pipefd[2], int flags) noexcept;

/**
 * @brief send message on socket
 * 
 * @throw SocketException on @code ECONNRESET EDESTADDRREQ EISCONN EMSGSIZE ENOTCONN EPIPE
 * @throw ResourceLimitException on @code ENOMEM
 */
extern ::ssize_t sendmsg(int sockfd, const struct ::msghdr *msg, int flags);

/**
 * @brief 
 * 
 * @throw ResourceLimitException on @code ENOMEM
 * @throw SocketException on @code ECONNREFUSED ENOTCONN
 */
extern ::ssize_t recvmsg(int sockfd, struct ::msghdr *msg, int flags);
}

#endif
