#include "support/ThreadPool.hpp"
#include "support/Log.h"

namespace netpp::support {

thread_local unsigned ThreadPool::threadId = 0;

ThreadPool::~ThreadPool()
{
	waitTask.notify_all();
	done = true;
	for (auto &t : threads)
	{
		if (t.joinable())
			t.join();
	}
}

ThreadPool::ThreadPool(unsigned threadNum)
	: done{false}, active{0}, taskCount{0}
{
	if (threadNum <= 0)
		threadNumber = std::thread::hardware_concurrency();
	else
		threadNumber = threadNum;
}

bool ThreadPool::start()
{
	try
	{
		for (unsigned i = 0; i < threadNumber; ++i)
			threads.emplace_back(std::thread(&ThreadPool::workerThread, this, i));
		return true;
	}
	catch (...)
	{
		waitTask.notify_all();
		done = true;
		return false;
	}
}

void ThreadPool::workerThread(unsigned id)
{
	threadId = id;
	while (!done)
	{
		if (workQueue.empty())
		{
			std::unique_lock lck(taskMutex);
			// maybe executing task will ThreadPool is desctruction, did not receive notify
			waitTask.wait_for(lck, std::chrono::microseconds(500));
		}
		runTask();
	}
}

void ThreadPool::runTask()
{
	try
	{
		TaskType task;
		if (workQueue.tryPop(task))
		{
			++active;
			--taskCount;
			task();
			--active;
		}
	}
	catch (...)
	{
		LOG_CRITICAL("Thread pool task throws an exeception");
		--active;
		throw;
	}
}

void ThreadPool::waitForDone(unsigned msec) const
{
	if (msec != 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(msec));
	else
	{
		while (taskCount != 0 || active != 0)
			std::this_thread::yield();
	}
}
}
