#include "support/ThreadPool.hpp"
#include "support/Log.h"

namespace netpp::support {

ThreadPool::~ThreadPool()
{
	waitTask.notify_all();
	m_quit = true;
	for (auto &t : threads)
	{
		if (t.joinable())
			t.join();
	}
}

ThreadPool::ThreadPool(unsigned threadNum)
	: m_quit{false}, m_activeThreads{0}, taskCount{0}
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
			threads.emplace_back(std::thread(&ThreadPool::workerThread, this));
		return true;
	}
	catch (...)
	{
		waitTask.notify_all();
		m_quit = true;
		return false;
	}
}

void ThreadPool::workerThread()
{
	while (!m_quit)
	{
		if (workQueue.empty())
		{
			std::unique_lock lck(m_waitTaskMutex);
			// maybe executing task will ThreadPool is destruction, did not receive notify
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
			++m_activeThreads;
			--taskCount;
			task();
			--m_activeThreads;
		}
	}
	catch (...)
	{
		LOG_CRITICAL("Thread pool task throws an exception");
		--m_activeThreads;
		throw;
	}
}

void ThreadPool::waitForDone(unsigned mSec) const
{
	if (mSec != 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(mSec));
	else
	{
		while (taskCount != 0 || m_activeThreads != 0)
			std::this_thread::yield();
	}
}
}
