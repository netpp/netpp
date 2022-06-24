#ifndef NETPP_RUNINLOOPHANDLER_H
#define NETPP_RUNINLOOPHANDLER_H

#include "epoll/EpollEventHandler.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <vector>

namespace netpp {
class EventLoop;
/**
 * @brief The functors that 'runs in loop' actually runs here,
 * every EventLoop contains an instance of this
 * 
 */
class RunInLoopHandler : public EpollEventHandler, public std::enable_shared_from_this<RunInLoopHandler> {
public:
	/**
	 * @brief Construct a RunInLoopHandler 
	 * 
	 * @throw ResourceLimitException when file descriptor reached the limitation
	 * 
	 */
	explicit RunInLoopHandler(EventLoop *loop);
	~RunInLoopHandler() override;

	/**
	 * @brief Add functor to run, and wake up event loop
	 * 
	 * @param functor Method to run
	 */
	void addPendingFunction(std::function<void()> functor);

protected:
	void handleIn() override;

	int fileDescriptor() const override { return m_wakeUpFd; }

private:
	int m_wakeUpFd;
	std::mutex m_functorMutex;
	std::vector<std::function<void()>> m_pendingFunctors;	// methods run in loop
};
}

#endif
