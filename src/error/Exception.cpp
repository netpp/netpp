#include "error/Exception.h"
#include "error/SocketError.h"
#include <cstring>
extern "C" {
#include <errno.h>
}

namespace netpp::error {
Exception::Exception(int errCode) noexcept
	: m_errnoCode{errCode}
{}

const char* Exception::what() const noexcept
{
	return std::strerror(m_errnoCode);
}

error::SocketError Exception::getErrorCode() noexcept
{
	return getError(m_errnoCode);
}
}
