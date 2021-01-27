#include "error/Exception.h"
#include "error/SocketError.h"
#include <cstring>
extern "C" {
#include <errno.h>
#include <string.h>
}

namespace netpp::error {
const char* ResourceLimitException::what() const noexcept
{
	return std::strerror(m_errnoCode);
}

error::SocketError ResourceLimitException::getSocketErrorCode() noexcept
{
	return getError(m_errnoCode);
}

const char* SocketException::what() const noexcept
{
	return std::strerror(m_errnoCode);
}

error::SocketError SocketException::getErrorCode() noexcept
{
	return getError(m_errnoCode);
}

const char* UnhandledSignal::what() const noexcept
{
	// TODO: list stacktrace
	return sigdescr_np(m_sigInfo.ssi_signo);
}
}
