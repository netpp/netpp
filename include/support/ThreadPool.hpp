#ifndef NETPP_THREAD_POOL_HPP
#define NETPP_THREAD_POOL_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <memory>
#include <type_traits>
#include "ThreadBase.hpp"

namespace netpp::support {
/**
 * @brief The thread pool
 * 
 */
class ThreadPool {
	using TaskType = internal::support::RunnableWrapper;
public:
	/**
	 * @brief Create a thread pool, threads will not be created before start() is called
	 * 
	 * @param threadNum if <= 0, use hardware thread number, else create n threads
	 */
	ThreadPool(unsigned threadNum = 0);
	~ThreadPool();

	/**
	 * @brief threads will not run untill call start()
	 * 
	 * @return true		if all threads start success
	 * @return false 	if one thread failed to start
	 */
	bool start();

	/// @brief get runnable return type
	template <typename Runnable, typename ... Args>
	using ResultType = typename std::result_of<Runnable(Args...)>::type;

	/**
	 * @brief add task to thread pool
	 * 
	 * @tparam Runnable		runnable objects
	 * @tparam Args			params
	 * @return std::future<RetType>		the future to get runnable return value
	 */
	template<typename Runnable, typename ... Args,
			typename RetType = ResultType<Runnable, Args...>,
			typename = typename std::enable_if<!std::is_same<RetType, void>::value>::type>
	std::future<RetType> run(Runnable runnable, Args ... args)
	{
		std::packaged_task<RetType(Args...)> task(runnable);
		std::future<RetType> res(task.get_future());
		TaskType wrapper(std::move(task), args...);
		++taskCount;
		workQueue.push(std::move(wrapper));
		waitTask.notify_one();
		return std::move(res);
	}

	/// @brief Explicit specialization for runnable void(...)
	template<typename Runnable, typename ... Args,
			typename RetType = ResultType<Runnable, Args...>,
			typename = typename std::enable_if<std::is_same<RetType, void>::value>::type>
	void run(Runnable runnable, Args ... args)
	{
		std::packaged_task<RetType(Args...)> task(runnable);
		TaskType wrapper(std::move(task), args...);
		++taskCount;
		workQueue.push(std::move(wrapper));
		waitTask.notify_one();
	}

	inline unsigned maxThreadCount() const { return threadNumber; }
	inline unsigned activeThreadCount() const { return m_activeThreads; }
	inline unsigned queuedTask() const { return taskCount; }

	/**
	 * @brief block this thread for milliseconds
	 * 
	 * @param msec pass 0 to block forever until all task is done
	 */
	void waitForDone(unsigned msec) const;

private:
	void workerThread(unsigned id);
	void runTask();

	std::atomic_bool m_quit;
	std::atomic_uint m_activeThreads;
	uint threadNumber;	// thread number will not change after constructed
	std::atomic_uint taskCount;

	std::mutex m_waitTaskMutex;
	std::condition_variable waitTask;

	internal::support::ThreadSafeQueue<TaskType> workQueue;
	std::vector<std::thread> threads;
	static thread_local unsigned threadId;
};
}

#endif