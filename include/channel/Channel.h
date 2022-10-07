//
// Created by gaojian on 22-7-18.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>
#include "support/Types.h"

namespace netpp {
class TransferBuffer;
class SocketConnectionHandler;
class ByteArray;
class ByteArrayPeeker;
class Channel {
public:
	virtual ~Channel() = default;

	/**
	 * @brief Tell event loop data on this channel is ready to send, event loop will send it later,
	 */
	virtual void send(const ByteArray &data) = 0;
	[[nodiscard]] virtual BufferLength readableBytes() const = 0;
	virtual ByteArrayPeeker peek() = 0;
	virtual ByteArray read() = 0;

	/**
	 * @brief Close the channel
	 */
	virtual void close();

	virtual void setMessageReceivedCallBack(const MessageReceivedCallBack &cb);
	virtual void setWriteCompletedCallBack(const WriteCompletedCallBack &cb);
	virtual void setDisconnectedCallBack(const DisconnectedCallBack &cb);
	virtual void setErrorCallBack(const ErrorCallBack &cb);

	virtual void setIdleTimeout(TimerInterval idleTime);

protected:
	std::weak_ptr<SocketConnectionHandler> _connection;
};
} // netpp

#endif //NETPP_CHANNEL_H
