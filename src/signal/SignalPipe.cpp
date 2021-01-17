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
		if (::close(m_signalPipe[0]) == -1 || ::close(m_signalPipe[1]) == -1)
		{
			// TODO: handle close error
			switch (errno)
			{
				case EBADF:
				case EINTR:
				case EIO:
				case ENOSPC:
				case EDQUOT:
				default:
					break;
			}
		}
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
		if (::write(m_signalPipe[1], &sig, sizeof(int)) == -1)
		{
			// TODO: handle write error
			switch (errno)
			{
				case EAGAIN:
				case EBADF:
				case EDESTADDRREQ:
				case EDQUOT:
				case EFAULT:
				case EFBIG:
				case EINTR:
				case EINVAL:
				case EIO:
				case ENOSPC:
				case EPERM:
				case EPIPE:
				default:
					break;
			}
		}
	}
	else
		SPDLOG_LOGGER_ERROR(logger, "Signal pipe not opened");
}

SignalPipe::SignalPipe()
{
	// singleton to avoid race condition
	if (::pipe2(m_signalPipe, O_NONBLOCK) == -1)
	{
		// TODO: handle pipe2 error
		switch (errno)
		{
			case EFAULT:
			case EINVAL:
			case EMFILE:
			case ENFILE:
			default:
				break;
		}
		SPDLOG_LOGGER_ERROR(logger, "Failed to open signal pipe");
	}
}
}
