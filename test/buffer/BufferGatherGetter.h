//
// Created by gaojian on 22-9-7.
//

#ifndef NETPP_BUFFERGATHERGETTER_H
#define NETPP_BUFFERGATHERGETTER_H

#include "buffer/BufferGather.h"
extern "C" {
#include <sys/socket.h>
}

inline std::size_t get_iovec_len(netpp::BufferGather &gather) { return gather.msghdr()->msg_iovlen; }
inline ::iovec *get_iovec(netpp::BufferGather &gather) { return gather.msghdr()->msg_iov; }

#endif //NETPP_BUFFERGATHERGETTER_H