//
// Created by gaojian on 2022/4/4.
//

#ifndef NETPP_EVENTLOOPMANAGER_H
#define NETPP_EVENTLOOPMANAGER_H

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <memory>
#include <unordered_map>

namespace netpp {
struct Config;
class EventLoop;
class RunInLoopHandler;
class TimeWheel;

/**
 * @brief A singleton managers ever EventLoop. The EventLoopManager will be initialized in Application
 */
class EventLoopManager {
public:
	/**
	 * @brief Create and init event loops with configure
	 * @param loopsCount the number of event loop runs
	 */
	explicit EventLoopManager(unsigned loopsCount);
	~EventLoopManager();

	/**
	 * @brief Dispatch an event loop, used generally when assign one more task to loop.
	 * @return The idea event loop
	 */
	EventLoop *dispatch();

	/**
	 * @brief Get all EventLoop managed, including main loop
	 * @return all event loops
	 */
	[[nodiscard]] std::vector<EventLoop *> loops() const;

	/**
	 * @brief Get the main event loop
	 * @return main event loop
	 */
	[[nodiscard]] EventLoop *mainLoop() const;

	/**
	 * @brief Start run all event loop in this dispatcher
	 * @note One loop will run in caller thread, would block caller
	 *
	 */
	void startLoop();

private:
	std::unique_ptr<EventLoop> m_mainEventLoop;
	using EventLoopVector = std::vector<std::unique_ptr<EventLoop>>;
	mutable std::mutex m_indexMutex;
	EventLoopVector::size_type m_dispatchIndex;    // guarded by m_indexMutex
	EventLoopVector m_loops;
	std::vector<std::thread> m_loopsThreads;
};
}

#endif //NETPP_EVENTLOOPMANAGER_H
