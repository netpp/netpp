#ifndef NETPP_STUB_SOCKET_H
#define NETPP_STUB_SOCKET_H

extern "C" {
#include <arpa/inet.h>
}

namespace netpp::internal::stub {
/**
 * @brief create a socket
 * 
 * @throw SocketException on (EACCES EAFNOSUPPORT)
 * @throw ResourceLimitException on (EMFILE ENFILE ENOBUFS ENOMEM)
 */
extern int socket(int domain, int type, int protocol);

/**
 * @brief bind to address
 * 
 * @throw SocketException on (EADDRINUSE)
 * @throw ResourceLimitException on (ENOMEM)
 */
extern int bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);

/**
 * @brief listen on socket
 * 
 * @throw SocketException on (EADDRINUSE)
 */
extern int listen(int sockfd, int backlog);

/**
 * @brief accept connection on socket
 * 
 * @throw SocketException on (ECONNABORTED)
 * @throw ResourceLimitException on (EMFILE ENFILE ENOBUFS ENOMEM EPERM)
 */
extern int accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags);

/**
 * @brief connect on a socket
 * 
 * @throw SocketException on (EADDRINUSE EADDRNOTAVAIL ECONNREFUSED EINPROGRESS ENETUNREACH ETIMEDOUT)
 */
extern int connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);

extern int shutdown(int sockfd, int how) noexcept;
extern int getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen) noexcept;
}

#endif
