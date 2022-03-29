//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_URIEXCEPTION_H
#define NETPP_URIEXCEPTION_H

#include <string>
#include "error/Exception.h"

namespace netpp::uri {
/**
 * @brief A single exception type to represent errors in parsing, encoding, and decoding URIs.
 *
 */
class UriException : public error::Exception {
public:
	explicit UriException(std::string msg) : m_msg(std::move(msg))
	{}

	[[nodiscard]] const char *what() const noexcept override
	{ return m_msg.c_str(); }

private:
	std::string m_msg;
};
}

#endif //NETPP_URIEXCEPTION_H
