//
// Created by gaojian on 22-7-18.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>
#include "support/Types.h"
#include "buffer/ByteArray.h"

namespace netpp {
class Channel {
public:
	virtual ~Channel() = default;

	/**
	 * @brief Tell event loop data on this channel is ready to send, event loop will send it later,
	 */
	virtual void send(const ByteArray &data) = 0;
	virtual ByteArray::LengthType readableBytes() const = 0;
	virtual ByteArray peek(ByteArray::LengthType size) = 0;
	virtual ByteArray read(ByteArray::LengthType size) = 0;

	/**
	 * @brief Close the channel
	 */
	virtual void close() = 0;

	virtual void setMessageReceivedCallBack(const MessageReceivedCallBack &cb) = 0;
	virtual void setWriteCompletedCallBack(const WriteCompletedCallBack &cb) = 0;
	virtual void setDisconnectedCallBack(const DisconnectedCallBack &cb) = 0;
	virtual void setErrorCallBack(const ErrorCallBack &cb) = 0;

	virtual void setIdleTimeout(TimerInterval idleTime) = 0;
};
} // netpp

#endif //NETPP_CHANNEL_H
