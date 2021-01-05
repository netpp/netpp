//
// Created by gaojian on 2020/8/9.
//

#ifndef NETPP_SOCKETEVENTHANDLER_H
#define NETPP_SOCKETEVENTHANDLER_H

#include <memory>
#include "epoll/EventHandler.h"
#include "Socket.h"
#include "ByteArray.h"
#include "Timer.h"
#include "Address.h"
#include "Events.h"

namespace netpp {
class EventLoopDispatcher;
class EventLoop;
}

namespace netpp::handlers {
// TODO: close acceptor
class Acceptor : public epoll::EventHandler {
public:
	Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<Socket> &&socket);
	~Acceptor() override = default;

	void listen();

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleDisconnect() override;
	static bool makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 std::unique_ptr<Events> &&eventsPrototype);

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<Socket> m_socket;
	std::unique_ptr<Events> m_events;
};

// TODO: close connector
class Connector : public epoll::EventHandler {
public:
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr <Socket> &&socket);
	~Connector() override = default;

	void connect();
	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleDisconnect() override;

	static bool makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  std::unique_ptr<Events> &&eventsPrototype);

private:
	void reconnect();

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<Socket> m_socket;
	std::unique_ptr<Timer> m_retryTimer;
	std::unique_ptr<Events> m_events;
};

enum class TcpState {
	Connecting, Connected, Disconnecting, Disconnected
};

class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(std::unique_ptr <Socket> &&socket, EventLoop *loop);
	~TcpConnection() override = default;

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleDisconnect() override;

	inline EventLoop *getConnectionLoop() { return _loop; }
	void sendInLoop();
	void closeAfterWriteCompleted();

	static std::shared_ptr<Channel> makeTcpConnection(EventLoop *loop, std::unique_ptr<Socket> &&socket,
									  std::unique_ptr<Events> &&eventsPrototype);

private:
	EventLoop *_loop;
	TcpState m_state;
	bool m_isWaitWriting;
	std::unique_ptr<Socket> m_socket;
	std::shared_ptr<ByteArray> m_writeBuffer;
	std::shared_ptr<ByteArray> m_receiveBuffer;
	std::unique_ptr<Events> m_events;
};
}

#endif //NETPP_SOCKETEVENTHANDLER_H
