//
// Created by gaojian on 2022/3/24.
//

#ifndef NETPP_HTTPCHANNEL_H
#define NETPP_HTTPCHANNEL_H

#include "Channel.h"

namespace netpp {
class ByteArray;
namespace internal::buffer {
class HttpChannelConversion;
}

class HttpChannel : public Channel {
	friend class internal::buffer::HttpChannelConversion;
public:
	using BufferConversion = internal::buffer::HttpChannelConversion;

	explicit HttpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection);

	std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() override;

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
	std::shared_ptr<ByteArray> m_receiveArray;
	std::shared_ptr<ByteArray> m_sendHeaderArray;
	std::shared_ptr<ByteArray> m_sendBodyArray;
};
}

#endif //NETPP_HTTPCHANNEL_H
