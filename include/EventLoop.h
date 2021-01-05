//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include "epoll/Epoll.h"
#include <unordered_set>
#include <functional>
#include <mutex>
#include <memory>

namespace netpp {
namespace epoll {
class EventHandler;
}
class EventLoop {
public:
	EventLoop() = default;
	~EventLoop();

	[[noreturn]] void run();
	void addEventHandlerToLoop(std::shared_ptr<epoll::EventHandler> handler);
	void removeEventHandlerFromLoop(std::shared_ptr<epoll::EventHandler> handler);

	/**
	 * @brief return this thread's event loop, only avaiable after run
	 */
	static inline EventLoop *thisLoop() { return _thisThreadLoop; }
	inline epoll::Epoll *getPoll() { return &m_poll; }

	void runInLoop(std::function<void()> functor);

private:
	static thread_local EventLoop *_thisThreadLoop;
	epoll::Epoll m_poll;
	std::unordered_set<std::shared_ptr<epoll::EventHandler>> m_handlers;

	std::mutex m_functorMutex;
	std::vector<std::function<void()>> m_pendingFuns;
};
}

#endif //NETPP_EVENTLOOP_H
