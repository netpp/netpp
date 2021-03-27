//
// Created by gaojian on 2020/7/7.
//

#ifndef NETPP_EVENTS_H
#define NETPP_EVENTS_H

#include <functional>
#include <memory>

namespace netpp {
class Channel;
namespace error {
enum class SocketError;
}
namespace signal {
enum class Signals;
}
namespace support {
class ThreadPool;
}
namespace internal {
/**
 * @brief test if class has method, only for Events
 * @param METHOD method name
 * @param ARG... method argument
 */
#define ASSERT_HAS_EVENT_METHOD(METHOD, ARG...)		\
template<typename T>				\
class has##METHOD {					\
public:								\
	template<typename U, void(U::*)(ARG) = &U::on##METHOD>			\
	static constexpr bool assert_has(U*) { return true; }			\
	static constexpr bool assert_has(...) { return false; }			\
	static constexpr bool value = assert_has(static_cast<T*>(0));	\
};
ASSERT_HAS_EVENT_METHOD(Connected, std::shared_ptr<netpp::Channel>)
ASSERT_HAS_EVENT_METHOD(MessageReceived, std::shared_ptr<netpp::Channel>)
ASSERT_HAS_EVENT_METHOD(WriteCompleted)
ASSERT_HAS_EVENT_METHOD(Disconnect)
ASSERT_HAS_EVENT_METHOD(Error, error::SocketError)
ASSERT_HAS_EVENT_METHOD(Signal, signal::Signals)
}

/**
 * @brief netpp events, callbacks.
 * 
 * user will define their event handler class to accept comming events,
 * who's methods should have same signature in this class.
 * 
 * User-defined handler mighit run in different thread, make sure it's 
 * thread safe.
 * 
 * @section Events you can handle
 * @see Events::onConnected
 * @see Events::onMessageReceived
 * @see Events::onWriteCompleted
 * @see Events::onDisconnect
 * @see Events::onError
 * @see Events::onSignal
 */
class Events final {
public:
	Events() : m_eventsPool{nullptr}, m_impl{nullptr} {}

	/**
	 * @brief Construct a new Events object
	 * 
	 * @tparam Impl		The user-defined event handler
	 * @param impl		shared_ptr to event handler, all event loop will shared same instance, make sure it's thread safe
	 * @param threads	threads event handler will use, <= 0 will run handler in loop thread
	 */
	template<typename Impl>
	explicit Events(std::shared_ptr<Impl> impl, int threads = 0)
	: m_impl{impl}
	{
		Impl *implPtr = static_cast<Impl *>(m_impl.get());
		if constexpr (internal::hasConnected<Impl>::value)
			m_connectedCb = std::bind(&Impl::onConnected, implPtr, std::placeholders::_1);
		if constexpr (internal::hasMessageReceived<Impl>::value)
			m_receiveMsgCb = std::bind(&Impl::onMessageReceived, implPtr, std::placeholders::_1);
		if constexpr (internal::hasWriteCompleted<Impl>::value)
			m_writeCompletedCb = std::bind(&Impl::onWriteCompleted, implPtr);
		if constexpr (internal::hasDisconnect<Impl>::value)
			m_disconnectCb = std::bind(&Impl::onDisconnect, implPtr);
		if constexpr (internal::hasError<Impl>::value)
			m_errorCb = std::bind(&Impl::onError, implPtr, std::placeholders::_1);
		if constexpr (internal::hasSignal<Impl>::value)
			m_signalCb = std::bind(&Impl::onSignal, implPtr, std::placeholders::_1);
		initThread(threads);
	}

	/**
	 * @brief handle connected event, triggered when a tcp connection just established
	 * 
	 * @param channel	provide a way sending data to pear, as connection just established, 
	 * 					the buffer should by empty, and not readable
	 */
	void onConnected(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief handle received message, triggered when pear send some data
	 * 
	 * @param channel	to read or write connection
	 */
	void onMessageReceived(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief triggered all data in buffer has wrote out
	 * 
	 */
	void onWriteCompleted();

	/**
	 * @brief handle disconnected, triggered when a tcp connection has being closed
	 * 
	 */
	void onDisconnect();

	/**
	 * @brief triggered when some error occurred
	 * 
	 * @param code		the error code
	 */
	// TODO: define and list what error can be handled here
	void onError(error::SocketError code);

	/**
	 * @brief if signal watcher is enabled, watched signals will be handled here
	 * 
	 * @param signal	the signal number
	 */
	void onSignal(signal::Signals signal);

private:
	void initThread(int threadsCount);

	// every event handler will shared same thread pool
	std::shared_ptr<support::ThreadPool> m_eventsPool;

	std::function<void(std::shared_ptr<netpp::Channel>)> m_connectedCb;
	std::function<void(std::shared_ptr<netpp::Channel>)> m_receiveMsgCb;
	std::function<void()> m_writeCompletedCb;
	std::function<void()> m_disconnectCb;
	std::function<void(error::SocketError)> m_errorCb;
	std::function<void(signal::Signals)> m_signalCb;

	std::shared_ptr<void> m_impl;
};
}

#endif //NETPP_EVENTS_H
