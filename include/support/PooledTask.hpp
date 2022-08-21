//
// Created by gaojian on 2021/3/28.
//

#ifndef NETPP_POOLEDTASK_HPP
#define NETPP_POOLEDTASK_HPP

#include <memory>
#include <functional>

namespace netpp {
/**
 * @brief Wrap runnable, make it type less
 *
 */
class RunnableWrapper {
public:
	/**
	 * @brief Wrap a runnable
	 * @tparam Runnable Runnable type
	 * @tparam Args Args to runnable
	 * @param runnable runnable instance
	 * @param args runnable args instance
	 */
	template<typename Runnable, typename ... Args>
	explicit RunnableWrapper(Runnable runnable, Args &&... args)
			: m_runnable{std::make_unique<ImplType<Runnable, Args...>>(std::move(runnable), std::forward<Args>(args)...)}
	{}

	/**
	 * @brief Move from another wrapper
	 * @param other an other wrapper
	 */
	RunnableWrapper(RunnableWrapper &&other) noexcept
			: m_runnable{std::move(other.m_runnable)}
	{}

	/**
	 * @brief Can not copy
	 */
	RunnableWrapper(const RunnableWrapper &) = delete;

	/**
	 * @brief No runnable wrapped by default
	 */
	RunnableWrapper() = default;

	/**
	 * @brief Make RunnableWrapper can be called like method
	 */
	void operator()() const
	{
		m_runnable->call();
	}

	/**
	 * @brief Move from an other wrapper
	 * @param runnableWrapper wrapper
	 * @return this wrapper
	 */
	RunnableWrapper &operator=(RunnableWrapper &&runnableWrapper) noexcept
	{
		m_runnable = std::move(runnableWrapper.m_runnable);
		return *this;
	}

	/**
	 * @brief Can not copy
	 */
	RunnableWrapper &operator=(RunnableWrapper &) = delete;

public:
	/**
	 * @brief Wrap runnable and make it type less
	 */
	struct ImplBase {
		/**
		 * @brief Call runnable
		 */
		virtual void call() = 0;

		virtual ~ImplBase() = default;
	};

	/**
	 * @brief The implementation of wrapper
	 * @tparam Runnable Runnable type
	 * @tparam Args Runnable args type
	 */
	template<typename Runnable, typename ... Args>
	struct ImplType : ImplBase {
		/** @brief Argument less runnable */
		std::function<void()> r;
		/** @brief Real runnable */
		Runnable _runnable;

		/**
		 * @brief Construct a real wrapper
		 * @param runnable Runnable
		 * @param args Runnable args
		 */
		explicit ImplType(Runnable runnable, Args &&... args)
				: r{std::bind(&ImplType<Runnable, Args...>::runRunnable, this, std::forward<Args>(args)...)},
				  _runnable{std::move(runnable)}
		{}

		/**
		 * @brief Move from an other
		 * @param other other instance
		 */
		ImplType(ImplType &&other) noexcept
				: _runnable{std::move(other._runnable)}, r{std::move(other.r)}
		{}

		/**
		 * @brief Not copyable
		 */
		ImplType(ImplType &) = delete;

		/**
		 * @brief Not copyable
		 */
		ImplType(const ImplType &) = delete;

		/**
		 * @brief Not copyable
		 */
		ImplType &operator=(ImplType &) = delete;

		/**
		 * @brief Call runnable without args
		 */
		inline void call() override
		{ r(); }

		/**
		 * @brief Call runnable with args
		 * @param args args to runnable
		 */
		void runRunnable(Args ... args)
		{
			_runnable(args...);
		}
	};

	/**
	 * @brief Runnable implement
	 */
	std::unique_ptr<ImplBase> m_runnable;
};
}

#endif //NETPP_POOLEDTASK_HPP
