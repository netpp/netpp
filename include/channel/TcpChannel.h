//
// Created by gaojian on 22-7-18.
//

#ifndef NETPP_TCPCHANNEL_H
#define NETPP_TCPCHANNEL_H

#include "Channel.h"

namespace netpp {
class EventLoop;
class SocketDevice;
class TcpChannelImpl;
class TcpChannel : public Channel {
public:
	/**
	 * @brief Create a connection
	 * @param loop
	 * @param socket
	 */
	TcpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket);

	~TcpChannel() override;

	void send(const ByteArray &data) override;

	void close() override;

	void setMessageReceivedCallBack(const MessageReceivedCallBack &cb) override;
	void setWriteCompletedCallBack(const WriteCompletedCallBack &cb) override;
	void setDisconnectedCallBack(const DisconnectedCallBack &cb) override;
	void setErrorCallBack(const ErrorCallBack &cb) override;

private:
	std::shared_ptr<TcpChannelImpl> m_impl;
};
}

#endif //NETPP_TCPCHANNEL_H
