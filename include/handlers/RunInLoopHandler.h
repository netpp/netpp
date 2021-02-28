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
 * @brief The functors that 'runs in loop' actually runs here,
 * every EventLoop contains an instance of this
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

	void handleIn() override;

	/**
	 * @brief Add functor to run, and wake up event loop
	 * 
	 * @param functor Method to run
	 */
	void addPendingFunction(std::function<void()> functor);

	static std::shared_ptr<RunInLoopHandler> makeRunInLoopHandler(EventLoop *loop);

private:
	// TODO: maybe this wake up fd can put in event loop, for other class to use
	int m_wakeUpFd;
	std::mutex m_functorMutex;
	std::vector<std::function<void()>> m_pendingFuns;	// methods run in loop
};
}

#endif
