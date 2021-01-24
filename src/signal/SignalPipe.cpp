#include "signal/SignalPipe.h"
#include "Log.h"
#include "stub/IO.h"

namespace netpp::signal {
// FIXME: use signalfd instead of pipe
int SignalPipe::m_signalPipe[2] = {-1, -1};

SignalPipe::~SignalPipe()
{
	if (isPipeOpened())
	{
		if (stub::close(m_signalPipe[0]) == -1 || stub::close(m_signalPipe[1]) == -1)
			SPDLOG_LOGGER_WARN(logger, "failed to close signal pipe");
	}
}

bool SignalPipe::isPipeOpened()
{
	return (m_signalPipe[0] != -1 && m_signalPipe[1] != -1);
}

void SignalPipe::handleSignal(int sig)
{
	// No mutex acquired, m_signalPipe won't change during runtime
	// FIXME: if no SignalHandler constructed, will never read from pipe
	if (SignalPipe::instance().isPipeOpened())
	{
		if (stub::write(m_signalPipe[1], &sig, sizeof(int)) == -1)
			SPDLOG_LOGGER_WARN(logger, "signal failed write pipe");
	}
	else
		SPDLOG_LOGGER_ERROR(logger, "signal pipe not opened");
}

SignalPipe::SignalPipe() noexcept
{
	// singleton to avoid race condition
	if (stub::pipe2(m_signalPipe, O_NONBLOCK) == -1)
		SPDLOG_LOGGER_ERROR(logger, "failed to open signal pipe");
}
}
