#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "epoll/EpollEventHandler.h"
#include "support/Types.h"

namespace netpp {
class EventLoop;
enum class Signals;
/**
 * @brief The SignalHandler handles watched signals, and notify user
 * 
 */
class SignalHandler : public EpollEventHandler, public std::enable_shared_from_this<SignalHandler> {
public:
	/// @brief Use makeSignalHandler to create SignalHandler
	explicit SignalHandler(EventLoop *loop, const std::initializer_list<netpp::Signals> &interestedSignals);
	~SignalHandler() override;

	void setSignalCallback(SignalCallBack cb);

protected:
	/**
	 * @brief Handle read events on signal fd, triggered when
	 * 1.signal in watching set emits
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 *
	 */
	void handleIn() override;

	int fileDescriptor() const override { return m_signalFd; }

private:
	int m_signalFd;
	std::mutex m_callBackMutex;
	SignalCallBack m_callback;
};
}

#endif
