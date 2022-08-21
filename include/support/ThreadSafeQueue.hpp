#ifndef NETPP_THREAD_BASE_HPP
#define NETPP_THREAD_BASE_HPP

#include <mutex>
#include <queue>
#include <condition_variable>

namespace netpp {
/**
 * @brief The thread safe queue
 * 
 */
template<typename T>
class ThreadSafeQueue {
public:
	/**
	 * @brief Create a thread safe queue
	 */
	ThreadSafeQueue() = default;

	/**
	 * @brief Push an item to queue
	 * @param newValue item to push
	 */
	void push(T &&newValue)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		queue.push(std::move(newValue));
		conditionVariable.notify_one();
	}

	/**
	 * @brief Pop an item, if queue is empty, wait until new item pushed
	 * @param value store pop item
	 */
	void waitPop(T &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		conditionVariable.wait(lock, [this] { return !queue.empty(); });
		value = queue.front();
		queue.pop();
	}

	/**
	 * @brief Try to pop an item
	 * @param value store pop item
	 * @return true if queue is not empty and pop success, otherwise false
	 */
	bool tryPop(T &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (queue.empty())
			return false;
		value = std::move(queue.front());
		queue.pop();
		return true;
	}

	/**
	 * @brief Is queue empty
	 * @return true if empty, otherwise false
	 */
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