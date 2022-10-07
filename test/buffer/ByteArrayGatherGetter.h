//
// Created by gaojian on 22-9-7.
//

#ifndef NETPP_BYTEARRAYGATHERGETTER_H
#define NETPP_BYTEARRAYGATHERGETTER_H

#include "buffer/BufferGather.h"
extern "C" {
#include <sys/socket.h>
}

inline std::size_t get_iovec_len(netpp::ByteArrayGather &gather) { return gather.msghdr()->msg_iovlen; }
inline ::iovec *get_iovec(netpp::ByteArrayGather &gather) { return gather.msghdr()->msg_iov; }

#endif //NETPP_BYTEARRAYGATHERGETTER_H