#ifndef NETPP_THREAD_BASE_HPP
#define NETPP_THREAD_BASE_HPP

#include <mutex>
#include <queue>
#include <condition_variable>

namespace netpp::support {
/**
 * @brief The thread safe queue
 * 
 */
template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() = default;

	void push(T &&newValue)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		queue.push(std::move(newValue));
		conditionVariable.notify_one();
	}

	void waitPop(T &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		conditionVariable.wait(lock, [this] { return !queue.empty(); });
		value = queue.front();
		queue.pop();
	}

	bool tryPop(T &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (queue.empty())
			return false;
		value = std::move(queue.front());
		queue.pop();
		return true;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return queue.empty();
	}

private:
	mutable std::mutex m_mutex;
	std::condition_variable conditionVariable;
	std::queue<T> queue;
};
}

#endif