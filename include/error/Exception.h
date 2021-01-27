#ifndef NETPP_EXCEPTION_H
#define NETPP_EXCEPTION_H

#include <exception>
extern "C" {
#include <sys/signalfd.h>
}

namespace netpp::error {
enum class SocketError;
class Exception : public std::exception {
public:
	explicit Exception() noexcept = default;
	const char* what() const noexcept override = 0;
};

class ResourceLimitException : public Exception {
public:
	explicit ResourceLimitException(int errCode) noexcept : m_errnoCode{errCode} {}
	const char* what() const noexcept override;
	error::SocketError getSocketErrorCode() noexcept;
private:
	int m_errnoCode;
};

class SocketException : public Exception {
public:
	explicit SocketException(int errCode) noexcept : m_errnoCode{errCode} {}
	const char* what() const noexcept override;
	error::SocketError getErrorCode() noexcept;
private:
	int m_errnoCode;
};

class UnhandledSignal : public Exception {
public:
	explicit UnhandledSignal(::signalfd_siginfo sigInfo) noexcept : m_sigInfo{sigInfo} {}
	const char* what() const noexcept override;
private:
	::signalfd_siginfo m_sigInfo;
};
}

#endif
