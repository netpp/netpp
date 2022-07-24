//
// Created by gaojian on 22-7-18.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>
#include "support/Types.h"

namespace netpp {
class ByteArray;
class Channel {
public:
	virtual ~Channel() = default;

	virtual void send(const ByteArray &data) = 0;

	virtual void close() = 0;

	virtual void setMessageReceivedCallBack(const MessageReceivedCallBack &cb) = 0;
	virtual void setWriteCompletedCallBack(const WriteCompletedCallBack &cb) = 0;
	virtual void setDisconnectedCallBack(const DisconnectedCallBack &cb) = 0;
	virtual void setErrorCallBack(const ErrorCallBack &cb) = 0;
};
} // netpp

#endif //NETPP_CHANNEL_H
