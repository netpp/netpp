#ifndef NETPP_THREAD_BASE_HPP
#define NETPP_THREAD_BASE_HPP

#include <mutex>
#include <memory>
#include <functional>
#include <queue>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <future>

namespace netpp::internal::support {
class RunnableWrapper {
public:
	template<typename Runnable, typename ... Args>
	explicit RunnableWrapper(Runnable runnable, Args ... args)
			: m_runnable{std::make_unique<ImplType<Runnable, Args...>>(std::move(runnable), args...)}
	{}

	RunnableWrapper(RunnableWrapper &&other) noexcept
		: m_runnable{std::move(other.m_runnable)}
	{}

	RunnableWrapper(const RunnableWrapper &&other) = delete;
	RunnableWrapper(RunnableWrapper &) = delete;
	RunnableWrapper() = default;

	void operator()()
	{
		m_runnable->call();
	}

	RunnableWrapper &operator=(RunnableWrapper &&runnableWrapper) noexcept
	{
		m_runnable = std::move(runnableWrapper.m_runnable);
		return *this;
	}

	RunnableWrapper &operator=(RunnableWrapper &) = delete;

public:
	struct ImplBase {
		virtual void call() = 0;

		virtual ~ImplBase() = default;
	};

	template<typename Runnable, typename ... Args>
	struct ImplType : ImplBase {
		std::function<void()> r;
		Runnable _runnable;

		explicit ImplType(Runnable runnable, Args ... args)
				: r{std::bind(&ImplType<Runnable, Args...>::runRunnable, this, args...)}, _runnable{std::move(runnable)}
		{}

		ImplType(ImplType &&other) noexcept
			: _runnable{std::move(other._runnable)}, r{std::move(other.r)}
		{}

		ImplType(ImplType &) = delete;
		ImplType(const ImplType &) = delete;
		ImplType &operator=(ImplType &) = delete;

		inline void call() override
		{ r(); }

		void runRunnable(Args ... args)
		{
			_runnable(args...);
		}
	};

	std::unique_ptr<ImplBase> m_runnable;
};


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