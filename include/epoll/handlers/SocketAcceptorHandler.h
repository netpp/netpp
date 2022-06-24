#ifndef NETPP_SOCKETACCEPTORHANDLER_H
#define NETPP_SOCKETACCEPTORHANDLER_H

#include "epoll/EpollEventHandler.h"
#include "location/Address.h"
#include "support/Types.h"

namespace netpp {
class SocketDevice;
enum class TcpState;
/**
 * @brief The SocketAcceptorHandler receives new connection, and create SocketConnectionHandler object for each connection
 * 
 */
class SocketAcceptorHandler : public EpollEventHandler, public std::enable_shared_from_this<SocketAcceptorHandler> {
public:
	/**
	 * @brief Create an acceptor
	 *
	 * @param dispatcher				Event loop dispatcher, assign acceptor to an EventLoop
	 * @param listenAddr				The address acceptor should listen
	 * @param eventsPrototype			User-define event handler
	 * @return std::weak_ptr<SocketAcceptorHandler>	The acceptor just created
	 */
	SocketAcceptorHandler(EventLoop *loop, Address address);
	~SocketAcceptorHandler() override;

	/**
	 * @brief Start listen and watch connect event,
	 * can be used out side EventLoop, thread safe
	 * 
	 */
	void listen();

protected:
	/**
	 * @brief EPOLLIN triggered when new connection coming
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 *
	 */
	void handleIn() override;

	int fileDescriptor() const override;

private:
	std::unique_ptr<SocketDevice> m_socket;

	Address m_address;

	ConnectedCallBack m_connectedCallback;
	ErrorCallBack m_errorCallback;

	/**
	 * @brief The state of acceptor
	 * 
	 *                 (stop)
	 *             +-----<-----+
	 *             |           |
	 * * -----> Closed -----> Listen
	 *  (construct)   (listen)
	 */
	TcpState m_state;
};
}

#endif
