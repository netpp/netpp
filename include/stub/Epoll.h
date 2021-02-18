#ifndef NETPP_STUB_EPOLL_H
#define NETPP_STUB_EPOLL_H

struct epoll_event;
// struct sigset_t;

namespace netpp::internal::stub {
/**
 * @brief create an epoll fd
 * 
 * @throw ResourceLimitException on @code EMFILE ENFILE ENOMEM
 */
// extern int epoll_create(int size);
extern int epoll_create1(int flags);

/**
 * @brief control an epoll fd
 * 
 * @throw ResourceLimitException on @code ENOMEM ENOSPC
 */
extern int epoll_ctl(int epfd, int op, int fd, struct ::epoll_event *event);

extern int epoll_wait(int epfd, struct ::epoll_event *events, int maxevents, int timeout) noexcept;
// int epoll_pwait(int epfd, struct ::epoll_event *events, int maxevents, int timeout, const ::sigset_t *sigmask);
}
#endif
