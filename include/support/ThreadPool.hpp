#ifndef NETPP_THREAD_POOL_HPP
#define NETPP_THREAD_POOL_HPP

#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <memory>
#include "ThreadBase.hpp"

namespace netpp::support {
class ThreadPool {
	using TaskType = RunnableWrapper;
public:
	/**
	 * @brief Construct a new Thread Pool object
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

	template<typename Runnable, typename ... Args>
	std::future<typename std::result_of<Runnable(Args...)>::type> run(Runnable runnable, Args ... args)
	{
		using ResultType = typename std::result_of<Runnable(Args...)>::type;
		std::packaged_task<ResultType(Args...)> task(runnable);
		std::future<ResultType> res(task.get_future());
		RunnableWrapper wrapper(std::move(task), args...);
		++taskCount;
		workQueue.push(std::move(wrapper));
		waitTask.notify_one();
		return std::move(res);
	}

	inline unsigned maxThreadCount() const { return threadNumber; }
	inline unsigned activeThreadCount() const { return active; }
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

	std::atomic_bool done;
	std::atomic_uint active;
	std::atomic_uint threadNumber;
	std::atomic_uint taskCount;

	std::mutex taskMutex;
	std::condition_variable waitTask;

	ThreadSafeQueue<TaskType> workQueue;
	std::vector<std::thread> threads;
	static thread_local unsigned threadId;
};
}

#endif