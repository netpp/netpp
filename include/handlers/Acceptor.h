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
	 * @brief start listen and watch connect event,
	 * can be used out side EventLoop, thread safe
	 * 
	 */
	void listen();

	/**
	 * @brief stop accept incomming connections, remove Acceptor from EventLoop,
	 * can be used out side EventLoop, thread safe
	 */
	void stop();

	/**
	 * @brief handle read events on accept socket, triggered when
	 * 1.new connection comming
	 * @note handlers will run only in EventLoop, NOT thread safe
	 * 
	 */
	void handleRead() override;
	void handleWrite() override {};

	/**
	 * @brief handle epoll error, this may not triggered
	 * @note handlers will run only in EventLoop, NOT thread safe
	 * 
	 */
	void handleError() override;

	void handleClose() override {};

	/**
	 * @brief create a new acceptor, thread safe
	 * 
	 * @param dispatcher				event loop dispatcher, assign acceptor to an EventLoop
	 * @param listenAddr				the address acceptor should listen
	 * @param eventsPrototype			user-define event handler
	 * @return std::weak_ptr<Acceptor>	the acceptor just created
	 */
	static std::shared_ptr<Acceptor> makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 Events eventsPrototype);

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	Events m_events;	// user-defined event handler

	/**
	 * @brief the state of acceptor
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
