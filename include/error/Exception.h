#ifndef NETPP_EXCEPTION_H
#define NETPP_EXCEPTION_H

#include <exception>
#include <string>
#include "Error.h"

namespace netpp {
/**
 * @brief The interface of netpp exceptions
 * 
 */
class Exception : public std::exception {
public:
	/**
	 * @brief
	 */
	explicit Exception() noexcept = default;
	/**
	 * @brief Exception info
	 * @return string that tells what's wrong
	 */
	[[nodiscard]] const char* what() const noexcept override = 0;
};

/**
 * @brief The netpp internal exception
 * 
 */
class InternalException : public Exception {
public:
	/**
	 * @brief Create new socket exception
	 * @param errCode error code that indicates socket goes wrong
	 */
	explicit InternalException(Error errCode) noexcept : m_errCode{errCode} {}
	/**
	 * @brief Exception info
	 * @return string that tells what's wrong
	 */
	[[nodiscard]] const char* what() const noexcept override;
	/**
	 * @brief Get as socket error
	 * @return SocketError
	 */
	[[nodiscard]] Error getErrorCode() const noexcept { return m_errCode; }
private:
	Error m_errCode;
};

/**
 * @brief A single exception type to represent errors in parsing, encoding, and decoding URIs.
 *
 */
class UriException : public Exception {
public:
	explicit UriException(std::string msg) : m_msg(std::move(msg))
	{}

	[[nodiscard]] const char *what() const noexcept override
	{ return m_msg.c_str(); }

private:
	std::string m_msg;
};
}

#endif
