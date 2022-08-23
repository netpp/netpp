//
// Created by gaojian on 22-7-18.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>
#include "support/Types.h"
#include "buffer/ByteArray.h"

namespace netpp {
class Buffer;
class SocketConnectionHandler;
class Channel {
public:
	virtual ~Channel() = default;

	/**
	 * @brief Tell event loop data on this channel is ready to send, event loop will send it later,
	 */
	virtual void send(const ByteArray &data);
	virtual ByteArray::LengthType readableBytes() const;
	virtual ByteArray peek(ByteArray::LengthType size);
	virtual ByteArray read(ByteArray::LengthType size);

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
	std::weak_ptr<Buffer> _buffer;
	std::weak_ptr<SocketConnectionHandler> _connection;
};
} // netpp

#endif //NETPP_CHANNEL_H
