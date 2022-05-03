//
// Created by gaojian on 22-4-23.
//


#ifndef NETPP_MOCKSOCKET_H
#define NETPP_MOCKSOCKET_H

#include <gmock/gmock.h>
#include "internal/socket/Socket.h"

class MockSocket : public netpp::internal::socket::Socket {
public:
	MockSocket() : Socket(netpp::Address()) {}
	MOCK_METHOD(int, fd, ());
	MOCK_METHOD(void, listen, ());
	MOCK_METHOD(std::unique_ptr<Socket>, accept, ());
	MOCK_METHOD(void, connect, ());
	MOCK_METHOD(netpp::error::SocketError, getError, ());
	MOCK_METHOD(netpp::Address, getAddr, ());
	MOCK_METHOD(void, shutdownWrite, ());
};

#endif //NETPP_MOCKSOCKET_H
