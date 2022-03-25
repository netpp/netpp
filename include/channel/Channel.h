//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>

namespace netpp {
namespace internal {
namespace handlers {
class TcpConnection;
}
namespace buffer {
class ChannelBufferConversion;
}
}

/**
 * @brief A Channel is an implement of TcpConnection's buffer.
 */
class Channel {
	friend class internal::handlers::TcpConnection;
public:
	explicit Channel(std::weak_ptr<internal::handlers::TcpConnection> connection);

	/**
	 * @brief Tell event loop data on this channel is ready to send, event loop will send it later,
	 */
	virtual void send() final;

	/**
	 * @brief Close the channel
	 */
	virtual void close() final;

	/**
	 * @brief Get unique id for this channel
	 * @return -1 if the connection was closed
	 */
	virtual int channelId() final;

	/**
	 * @brief Write to this channel is effective only when connection was established,
	 * otherwise write to the channel will not send anything.
	 * @return true: connection was established
	 */
	[[nodiscard]] virtual bool channelActive() const final;

	virtual std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() = 0;

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
};
}

#endif //NETPP_CHANNEL_H
