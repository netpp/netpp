//
// Created by gaojian on 2021/3/28.
//

#ifndef NETPP_POOLEDTASK_HPP
#define NETPP_POOLEDTASK_HPP

#include <memory>
#include <functional>

namespace netpp::support {
/**
 * @brief Wrap runnable, make it type less
 *
 */
class RunnableWrapper {
public:
	template<typename Runnable, typename ... Args>
	explicit RunnableWrapper(Runnable runnable, Args &&... args)
			: m_runnable{std::make_unique<ImplType<Runnable, Args...>>(std::move(runnable), std::forward<Args>(args)...)}
	{}

	RunnableWrapper(RunnableWrapper &&other) noexcept
			: m_runnable{std::move(other.m_runnable)}
	{}

	RunnableWrapper(const RunnableWrapper &) = delete;

	RunnableWrapper() = default;

	void operator()() const
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

		explicit ImplType(Runnable runnable, Args &&... args)
				: r{std::bind(&ImplType<Runnable, Args...>::runRunnable, this, std::forward<Args>(args)...)},
				  _runnable{std::move(runnable)}
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
}

#endif //NETPP_POOLEDTASK_HPP
