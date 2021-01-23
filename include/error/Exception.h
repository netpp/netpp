#ifndef NETPP_EXCEPTION_H
#define NETPP_EXCEPTION_H

#include <exception>

namespace netpp::error {
enum class SocketError;
class Exception : public std::exception {
public:
	explicit Exception(int errCode) noexcept;
	const char* what() const noexcept override;
	error::SocketError getErrorCode() noexcept;

private:
	int m_errnoCode;
};

class ResourceLimitException : public Exception {
public:
	explicit ResourceLimitException(int errCode) noexcept : Exception(errCode) {}
};

class SocketException : public Exception {
public:
	explicit SocketException(int errCode) noexcept : Exception(errCode) {}
};
}

#endif
