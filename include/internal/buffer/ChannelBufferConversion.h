//
// Created by kojiko on 2022/3/25.
//

#ifndef NETPP_CHANNELBUFFERCONVERSION_H
#define NETPP_CHANNELBUFFERCONVERSION_H

#include <memory>

namespace netpp {
class Channel;
namespace internal {
namespace socket {
class ByteArray2IOVec;
}
namespace buffer {
/**
* @brief Convert channel internal buffer to iovec.
 * The Channel should hide the implement of buffer, so as the way for socket to read/write buffer.
 *
*/
class ChannelBufferConversion {
public:
	virtual ~ChannelBufferConversion() = default;

	virtual std::unique_ptr<internal::socket::ByteArray2IOVec> readBufferConvert(Channel *channel) = 0;

	virtual std::unique_ptr<internal::socket::ByteArray2IOVec> writeBufferConvert(Channel *channel) = 0;
};

/**
 * @brief Support for TcpChannel, convert buffer for socket to read/write
 */
class TcpChannelConversion : public ChannelBufferConversion {
public:
	~TcpChannelConversion() override = default;

	/**
	* @brief Read from buffer, and send to peer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<internal::socket::ByteArray2IOVec> readBufferConvert(Channel *channel) override;

	/**
	* @brief Write peer message to buffer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<internal::socket::ByteArray2IOVec> writeBufferConvert(Channel *channel) override;
};
}
}
}

#endif //NETPP_CHANNELBUFFERCONVERSION_H
