#ifndef NETPP_RUNINLOOPHANDLER_H
#define NETPP_RUNINLOOPHANDLER_H

#include "epoll/EventHandler.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <vector>

namespace netpp {
class EventLoop;
}

namespace netpp::internal::handlers {
/**
 * @brief Handle run in loop problems
 * 
 */
class RunInLoopHandler : public epoll::EventHandler, public std::enable_shared_from_this<RunInLoopHandler> {
public:
	/**
	 * @brief Construct a RunInLoopHandler 
	 * 
	 * @throw ResourceLimitException when file descriptor reached the limitation
	 * 
	 */
	explicit RunInLoopHandler(EventLoop *loop);
	~RunInLoopHandler();

	void handleRead() override;
	void handleWrite() override {};
	void handleError() override {};
	void handleClose() override {};

	/**
	 * @brief Add functor to run, and wake up event loop
	 * 
	 * @param functor Method to run
	 */
	void addPendingFunction(std::function<void()> functor);

	static std::shared_ptr<RunInLoopHandler> makeRunInLoopHandler(EventLoop *loop);

private:
	int m_wakeUpFd[2];
	std::atomic_flag m_waitingWakeUp;
	std::mutex m_functorMutex;
	std::vector<std::function<void()>> m_pendingFuns;	// methods run in loop
};
}

#endif
