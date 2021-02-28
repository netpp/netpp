#ifndef NETPP_ACCEPTOR_H
#define NETPP_ACCEPTOR_H

#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "Events.h"
#include "Address.h"
#include "socket/SocketEnums.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::internal::handlers {
/**
 * @brief The Acceptor recieves new connection, and create TcpConnection object for each connection
 * 
 */
class Acceptor : public epoll::EventHandler, public std::enable_shared_from_this<Acceptor> {
public:
	/// @brief Use makeAcceptor to create an Acceptor
	Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket);
	~Acceptor() override = default;

	/**
	 * @brief Start listen and watch connect event,
	 * can be used out side EventLoop, thread safe
	 * 
	 */
	void listen();

	/**
	 * @brief Stop accept incomming connections, remove Acceptor from EventLoop,
	 * can be used out side EventLoop, thread safe
	 */
	void stop();

	/**
	 * @brief EPOLLIN triggered when new connection comming
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 * 
	 */
	void handleIn() override;

	/**
	 * @brief Create a new acceptor, thread safe
	 * 
	 * @param dispatcher				Event loop dispatcher, assign acceptor to an EventLoop
	 * @param listenAddr				The address acceptor should listen
	 * @param eventsPrototype			User-define event handler
	 * @return std::weak_ptr<Acceptor>	The acceptor just created
	 */
	static std::shared_ptr<Acceptor> makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 Events eventsPrototype);

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	Events m_events;	// user-defined event handler

	/**
	 * @brief The state of acceptor
	 * 
	 *             stop()
	 *         +-----<-----+
	 *         |           |
	 * * -> Closed -> Established
	 *  make     listen()     
	 */
	socket::TcpState m_state;
};
}

#endif
