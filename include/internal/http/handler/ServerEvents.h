//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_SERVEREVENTS_H
#define NETPP_SERVEREVENTS_H

#include <memory>

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
class ServerEvents {
public:
	/**
	 * @brief Handle connected event, triggered when a tcp connection just established
	 *
	 * @param channel	Provide a way sending data to pear, as connection just established,
	 * 					the buffer should by empty, and not readable
	 */
	void onConnected(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle received message, triggered when pear send some data
	 *
	 * @param channel	To read or write connection
	 */
	void onMessageReceived(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Triggered all data in buffer has wrote out
	 *
	 */
	void onWriteCompleted(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle disconnected, triggered when a tcp connection has being closed
	 *
	 */
	void onDisconnect(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Triggered when some error occurred
	 *
	 * @param code		The error code
	 */
	void onError(error::SocketError code);

	/**
	 * @brief If signal watcher is enabled, watched signals will be handled here
	 *
	 * @param signal	The signal number
	 */
	void onSignal(signal::Signals signal);
};
}
}

#endif //NETPP_SERVEREVENTS_H
