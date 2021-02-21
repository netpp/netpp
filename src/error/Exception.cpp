#include "error/Exception.h"
#include "error/SocketError.h"
#include "signal/Signals.h"
#include <cstring>

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

/*UnhandledSignal::UnhandledSignal(::signalfd_siginfo sigInfo) noexcept
	: m_sigInfo{sigInfo}
{
#if !__GLIBC_PREREQ(2, 32)
	signalStrBeforeGlibc2_32 = new char[signalStrLength];
#endif
}

UnhandledSignal::~UnhandledSignal()
{
#if !__GLIBC_PREREQ(2, 32)
	delete signalStrBeforeGlibc2_32;
#endif
}

const char* UnhandledSignal::what() const noexcept
{
	// TODO: list stacktrace
#if __GLIBC_PREREQ(2, 32)
	return sigdescr_np(m_sigInfo.ssi_signo);
#else
	// strsignal is not thread safe
	static std::mutex mutex;
	std::lock_guard lck(mutex);
	char *sig = strsignal(m_sigInfo.ssi_signo);
	std::memcpy(signalStrBeforeGlibc2_32, sig, signalStrLength);
	return signalStrBeforeGlibc2_32;
#endif
}

signal::Signals UnhandledSignal::signal()
{
	return signal::toNetppSignal(m_sigInfo.ssi_signo);
}*/
}
