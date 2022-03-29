//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_CLIENTEVENTS_H
#define NETPP_CLIENTEVENTS_H

#include <memory>
#include <queue>
#include "http/HttpRequest.h"

namespace netpp {
class Channel;
namespace error {
enum class SocketError;
}
namespace signal {
enum class Signals;
}
namespace internal::http::handler {
/**
 * @brief Handle server side http connection events
 */
class ClientEvents {
public:
	/**
	 * @brief Handle connected event
	 *
	 * @param channel	Provide a way sending data to pear, as connection just established,
	 * 					the buffer should by empty, and not readable
	 */
	void onConnected(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle message received event
	 *
	 * @param channel	To read or write connection
	 */
	void onMessageReceived(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle write complete event
	 *
	 */
	void onWriteCompleted(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle disconnected event
	 *
	 */
	void onDisconnect(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle error event
	 *
	 * @param code		The error code
	 */
	void onError(error::SocketError code);

	/**
	 * @brief Handle signal event
	 *
	 * @param signal	The signal number
	 */
	void onSignal(signal::Signals signal);

private:
	std::queue<std::unique_ptr<netpp::http::HttpBaseRequest>> m_pendingSend;
};
}
}

#endif //NETPP_CLIENTEVENTS_H
