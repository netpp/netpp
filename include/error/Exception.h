#ifndef NETPP_EXCEPTION_H
#define NETPP_EXCEPTION_H

#include <exception>

namespace netpp::error {
enum class SocketError;
/**
 * @brief The interface of netpp exceptions
 * 
 */
class Exception : public std::exception {
public:
	explicit Exception() noexcept = default;
	[[nodiscard]] const char* what() const noexcept override = 0;
};

/**
 * @brief Resources is limited, for example memory not enough, fd limitation
 * 
 */
class ResourceLimitException : public Exception {
public:
	explicit ResourceLimitException(int errCode) noexcept : m_errnoCode{errCode} {}
	[[nodiscard]] const char* what() const noexcept override;
	error::SocketError getSocketErrorCode() const noexcept;
private:
	int m_errnoCode;
};

/**
 * @brief Error happened on socket
 * 
 */
class SocketException : public Exception {
public:
	explicit SocketException(int errCode) noexcept : m_errnoCode{errCode} {}
	[[nodiscard]] const char* what() const noexcept override;
	error::SocketError getErrorCode() const noexcept;
private:
	int m_errnoCode;
};
}

#endif
