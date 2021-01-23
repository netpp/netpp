#ifndef NETPP_STUB_SOCKET_H
#define NETPP_STUB_SOCKET_H

extern "C" {
#include <arpa/inet.h>
}

namespace netpp::stub {
/**
 * @brief create a socket
 * 
 * @throw SocketException on @code EACCES EAFNOSUPPORT
 * @throw ResourceLimitException on @code EMFILE ENFILE ENOBUFS ENOMEM
 */
int socket(int domain, int type, int protocol);

/**
 * @brief bind to address
 * 
 * @throw SocketException on @code EADDRINUSE
 * @throw ResourceLimitException on @code ENOMEM
 */
int bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);

/**
 * @brief listen on socket
 * 
 * @throw SocketException on @code EADDRINUSE
 */
int listen(int sockfd, int backlog);

/**
 * @brief accept connection on socket
 * 
 * @throw SocketException on @code ECONNABORTED
 * @throw ResourceLimitException on @code EMFILE ENFILE ENOBUFS ENOMEM EPERM
 */
int accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags);

/**
 * @brief connect on a socket
 * 
 * @throw SocketException on @code EADDRINUSE EADDRNOTAVAIL ECONNREFUSED EINPROGRESS ENETUNREACH ETIMEDOUT
 */
int connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);

int shutdown(int sockfd, int how) noexcept;
int getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen) noexcept;
}

#endif
