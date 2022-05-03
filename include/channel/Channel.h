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
 *
 * For user, Channel must provide a way to access buffer, data can be read/write into buffer
 * For tcp connection, Channel is buffer.
 */
class Channel {
	friend class internal::handlers::TcpConnection;
public:
	/**
	 * @brief Create a channel
	 * @param connection Connecting bind to this channel
	 */
	explicit Channel(std::weak_ptr<internal::handlers::TcpConnection> connection);
	virtual ~Channel();

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

	/**
	 * @brief A Channel is for user, should not expose implementation of low level io,
	 * ChannelBufferConversion is support class to do the transformation. Each Channel's
	 * subclass should implement it's ChannelBufferConversion object.
	 * @return The specified ChannelBufferConversion for Channel
	 */
	virtual std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() = 0;

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
};
}

#endif //NETPP_CHANNEL_H
