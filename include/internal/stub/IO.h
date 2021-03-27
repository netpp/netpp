#ifndef NETPP_STUB_IO_H
#define NETPP_STUB_IO_H

extern "C" {
#include <unistd.h>
#include <sys/eventfd.h>
}

struct iovec;
struct msghdr;

namespace netpp::internal::stub {
extern int close(int fd) noexcept;
extern ::ssize_t write(int fd, const void *buf, ::size_t count) noexcept;
extern ::ssize_t read(int fd, void *buf, ::size_t count) noexcept;

/**
 * @brief Create a pipe
 * 
 * @throw ResourceLimitException on (EMFILE, ENFILE)
 */
extern int pipe2(int pipefd[2], int flags);

/**
 * @brief Send message on socket
 * 
 * @throw SocketException on (ECONNRESET EDESTADDRREQ EISCONN EMSGSIZE ENOTCONN EPIPE)
 * @throw ResourceLimitException on (ENOMEM)
 */
extern ::ssize_t sendmsg(int sockfd, const struct ::msghdr *msg, int flags);

/**
 * @brief Receive message from socket
 * 
 * @throw ResourceLimitException on (ENOMEM)
 * @throw SocketException on (ECONNREFUSED ENOTCONN)
 */
extern ::ssize_t recvmsg(int sockfd, struct ::msghdr *msg, int flags);

/**
 * @brief Create event fd
 * 
 * @throw ResourceLimitException on (EMFILE, ENFILE, ENODEV, ENOMEM)
 */
extern int eventfd(unsigned int initval, int flags);
extern int eventfd_read(int fd, ::eventfd_t *value);
extern int eventfd_write(int fd, ::eventfd_t value);
}

#endif
