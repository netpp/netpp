#include "signal/SignalPipe.h"
#include "Log.h"
extern "C" {
#include <unistd.h>
#include <fcntl.h>
}

namespace netpp::signal {
int SignalPipe::m_signalPipe[2] = {-1, -1};

SignalPipe::~SignalPipe()
{
	if (isPipeOpened())
	{
		::close(m_signalPipe[0]);
		::close(m_signalPipe[1]);
	}
}

bool SignalPipe::isPipeOpened()
{
	return (m_signalPipe[0] != -1 && m_signalPipe[1] != -1);
}

void SignalPipe::handleSignal(int sig)
{
	// No mutex acquired, m_signalPipe won't change during runtime
	if (SignalPipe::instance().isPipeOpened())
		::write(m_signalPipe[1], &sig, sizeof(int));
	else
		SPDLOG_LOGGER_ERROR(logger, "Signal pipe not opened");
}

SignalPipe::SignalPipe()
{
	// singleton to avoid race condition
	if (::pipe2(m_signalPipe, O_NONBLOCK) == -1)
	{
		// TODO: may be throw exception
		SPDLOG_LOGGER_ERROR(logger, "Failed to open signal pipe");
	}
}
}
