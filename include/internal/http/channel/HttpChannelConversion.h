//
// Created by gaojian on 2022/3/27.
//

#ifndef NETPP_HTTPCHANNELCONVERSION_H
#define NETPP_HTTPCHANNELCONVERSION_H

#include "internal/buffer/ChannelBufferConversion.h"

namespace netpp::internal::http::channel {
/**
 * @brief Support for HttpChannel, convert buffer for socket to read/write
 */
class HttpChannelConversion : public internal::buffer::ChannelBufferConversion {
public:
	~HttpChannelConversion() override = default;

	/**
	* @brief Read from buffer, and send to peer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr <internal::socket::ByteArray2IOVec> readBufferConvert(Channel *channel) override;

	/**
	* @brief Write peer message to buffer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr <internal::socket::ByteArray2IOVec> writeBufferConvert(Channel *channel) override;
};
}

#endif //NETPP_HTTPCHANNELCONVERSION_H
