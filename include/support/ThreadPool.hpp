#ifndef NETPP_THREAD_POOL_HPP
#define NETPP_THREAD_POOL_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <memory>
#include <type_traits>
#include "PooledTask.hpp"

namespace netpp {
template <typename T>
class ThreadSafeQueue;
/**
 * @brief The thread pool
 * 
 */
class ThreadPool {
	using TaskType = RunnableWrapper;
public:
	/**
	 * @brief Create a thread pool, threads will not be created before start() is called
	 * 
	 * @param ThreadNum if <= 0, use hardware thread number, else create n threads
	 */
	explicit ThreadPool(unsigned threadNum = 0);
	~ThreadPool();

	/**
	 * @brief Threads will not run until call start()
	 * 
	 * @return true		If all threads start success
	 * @return false 	If one thread failed to start
	 */
	bool start();

	/// @brief Get runnable return type
	template <typename Runnable, typename ... Args>
	using ResultType = typename std::result_of<Runnable(Args...)>::type;

	/**
	 * @brief Add task to thread pool
	 * 
	 * @tparam Runnable		Runnable objects
	 * @tparam Args			Params to functor
	 * @return std::future<RetType>		The future to get runnable return value
	 */
	template<typename Runnable, typename ... Args,
			typename RetType = ResultType<Runnable, Args...>,
			typename = typename std::enable_if<!std::is_same<RetType, void>::value>::type>
	std::future<RetType> run(Runnable runnable, Args && ... args)
	{
		std::packaged_task<RetType(Args...)> task(runnable);
		std::future<RetType> res(task.get_future());
		TaskType wrapper(std::move(task), std::forward<Args>(args)...);
		addTask(std::move(wrapper));
		return std::move(res);
	}

	/// @brief Explicit specialization for runnable void(...)
	template<typename Runnable, typename ... Args,
			typename RetType = ResultType<Runnable, Args...>,
			typename = typename std::enable_if<std::is_same<RetType, void>::value>::type>
	std::future<void> run(Runnable runnable, Args && ... args)
	{
		std::packaged_task<RetType(Args...)> task(runnable);
		std::future<void> res(task.get_future());
		TaskType wrapper(std::move(task), std::forward<Args>(args)...);
		addTask(std::move(wrapper));
		return res;
	}

	/**
	 * @brief	How many threads this thread pool created
	 */
	[[nodiscard]] inline unsigned maxThreadCount() const { return threadNumber; }

	/**
	 * @brief	How many threads running task in this moment
	 */
	[[nodiscard]] inline unsigned activeThreadCount() const { return m_activeThreads; }

	/**
	 * @brief	How many task queued in loop
	 */
	[[nodiscard]] inline unsigned queuedTask() const { return taskCount; }

	/**
	 * @brief Block this thread for milliseconds
	 * 
	 * @param mSec	Pass 0 to block forever until all task is done
	 */
	void waitForDone(unsigned mSec) const;

private:
	void workerThread();
	void runTask();
	void addTask(TaskType &&task);

	std::atomic_bool m_quit;
	std::atomic_uint m_activeThreads;
	uint threadNumber;	// thread number will not change after constructed
	std::atomic_uint taskCount;

	std::mutex m_waitTaskMutex;
	std::condition_variable waitTask;

	std::unique_ptr<ThreadSafeQueue<TaskType>> workQueue;
	std::vector<std::thread> threads;
};
}

#endif