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
 * @brief Resources is limited
 * @example
 * - memory not enough
 * - fd limitation
 */
class ResourceLimitException : public Exception {
public:
	/**
	 * @brief Create new resource exception
	 * @param errCode error code that indicates limited
	 */
	explicit ResourceLimitException(int errCode) noexcept : m_errnoCode{errCode} {}
	/**
	 * @brief Exception info
	 * @return string that tells what's wrong
	 */
	[[nodiscard]] const char* what() const noexcept override;
	/**
	 * @brief Get as socket error
	 * @return SocketError
	 */
	[[nodiscard]] error::SocketError getSocketErrorCode() const noexcept;
private:
	int m_errnoCode;
};

/**
 * @brief Error happened on socket
 * 
 */
class SocketException : public Exception {
public:
	/**
	 * @brief Create new socket exception
	 * @param errCode error code that indicates socket goes wrong
	 */
	explicit SocketException(int errCode) noexcept : m_errnoCode{errCode} {}
	/**
	 * @brief Exception info
	 * @return string that tells what's wrong
	 */
	[[nodiscard]] const char* what() const noexcept override;
	/**
	 * @brief Get as socket error
	 * @return SocketError
	 */
	[[nodiscard]] error::SocketError getErrorCode() const noexcept;
private:
	int m_errnoCode;
};
}

#endif
