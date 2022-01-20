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
#define NETPP_ASSERT_HAS_EVENT_METHOD(METHOD, ARG...)    \
template<typename T>                    \
class namingMatch##METHOD {                \
public:                                    \
    template<typename U, typename M = decltype(&U::on##METHOD)>                    \
    static constexpr std::true_type assert_has(U*) { return std::true_type(); }    \
    static constexpr std::false_type assert_has(...) { return std::false_type(); }    \
    static constexpr bool value = assert_has(static_cast<T*>(0));                \
};                                    \
template<typename T>                \
class fullMatch##METHOD {            \
public:                                \
    template<typename U, void(U::*)(ARG) = &U::on##METHOD>            \
    static constexpr std::true_type assert_has(U*) { return std::true_type(); }        \
    static constexpr std::false_type assert_has(...) { return std::false_type(); }    \
    static constexpr bool value = assert_has(static_cast<T*>(0));    \
};

NETPP_ASSERT_HAS_EVENT_METHOD(Connected, std::shared_ptr<netpp::Channel>)
NETPP_ASSERT_HAS_EVENT_METHOD(MessageReceived, std::shared_ptr<netpp::Channel>)
NETPP_ASSERT_HAS_EVENT_METHOD(WriteCompleted, std::shared_ptr<netpp::Channel>)
NETPP_ASSERT_HAS_EVENT_METHOD(Disconnect, std::shared_ptr<netpp::Channel>)
NETPP_ASSERT_HAS_EVENT_METHOD(Error, error::SocketError)
NETPP_ASSERT_HAS_EVENT_METHOD(Signal, signal::Signals)

#define NETPP_BIND_METHOD(METHOD, Impl, functor, binder)			\
if constexpr (internal::namingMatch##METHOD<Impl>::value) {	\
	static_assert(internal::fullMatch##METHOD<Impl>::value, "on"#METHOD"() method was defined, but mismatch params, try check your arguments");	\
	(functor) = (binder);	\
}
}

/**
 * @brief Netpp events, callbacks.
 * 
 * User will define their event handler class to accept coming events,
 * who's methods should have same signature in this class.
 * 
 * User-defined handler might run in different thread, make sure it's
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
	using ConnectedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
	using MessageReceivedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
	using WriteCompletedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
	using DisconnectedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
	using ErrorCallBack = std::function<void(error::SocketError)>;
	using SignalCallBack = std::function<void(signal::Signals)>;

public:
	Events() : m_eventsPool{nullptr}, m_impl{nullptr} {}

	/**
	 * @brief Construct a new Events object
	 * 
	 * @tparam Impl		The user-defined event handler
	 * @param impl		shared_ptr to event handler, all event loop will shared same instance, make sure it's thread safe
	 * @param threads	Threads event handler will use, <= 0 will run handler in loop thread
	 */
	template<typename Impl>
	explicit Events(std::shared_ptr<Impl> impl, int threads = 0)
	: m_eventsPool{nullptr}, m_impl{impl}
	{
		Impl *implPtr = static_cast<Impl *>(m_impl.get());
		NETPP_BIND_METHOD(Connected, Impl, m_connectedCb, std::bind(&Impl::onConnected, implPtr, std::placeholders::_1))
		NETPP_BIND_METHOD(MessageReceived, Impl, m_receiveMsgCb, std::bind(&Impl::onMessageReceived, implPtr, std::placeholders::_1))
		NETPP_BIND_METHOD(WriteCompleted, Impl, m_writeCompletedCb, std::bind(&Impl::onWriteCompleted, implPtr, std::placeholders::_1))
		NETPP_BIND_METHOD(Disconnect, Impl, m_disconnectCb, std::bind(&Impl::onDisconnect, implPtr, std::placeholders::_1))
		NETPP_BIND_METHOD(Error, Impl, m_errorCb, std::bind(&Impl::onError, implPtr, std::placeholders::_1))
		NETPP_BIND_METHOD(Signal, Impl, m_signalCb, std::bind(&Impl::onSignal, implPtr, std::placeholders::_1))
		initThread(threads);
	}

	void bindConnectedCallback(ConnectedCallBack cb);
	void bindMessageReceivedCallback(MessageReceivedCallBack cb);
	void bindWriteCompletedCallback(WriteCompletedCallBack cb);
	void bindDisconnectedCallback(DisconnectedCallBack cb);
	void bindErrorCallback(ErrorCallBack cb);
	void bindSignalCallback(SignalCallBack cb);

	/**
	 * @brief Handle connected event, triggered when a tcp connection just established
	 * 
	 * @param channel	Provide a way sending data to pear, as connection just established,
	 * 					the buffer should by empty, and not readable
	 */
	void onConnected(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle received message, triggered when pear send some data
	 * 
	 * @param channel	To read or write connection
	 */
	void onMessageReceived(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Triggered all data in buffer has wrote out
	 * 
	 */
	void onWriteCompleted(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Handle disconnected, triggered when a tcp connection has being closed
	 * 
	 */
	void onDisconnect(const std::shared_ptr<netpp::Channel> &channel);

	/**
	 * @brief Triggered when some error occurred
	 * 
	 * @param code		The error code
	 */
	// TODO: define and list what error can be handled here
	void onError(error::SocketError code);

	/**
	 * @brief If signal watcher is enabled, watched signals will be handled here
	 * 
	 * @param signal	The signal number
	 */
	void onSignal(signal::Signals signal);

private:
	void initThread(int threadsCount);

	// every event handler will shared same thread pool
	std::shared_ptr<support::ThreadPool> m_eventsPool;

	std::function<void(std::shared_ptr<netpp::Channel>)> m_connectedCb;
	std::function<void(std::shared_ptr<netpp::Channel>)> m_receiveMsgCb;
	std::function<void(std::shared_ptr<netpp::Channel>)> m_writeCompletedCb;
	std::function<void(std::shared_ptr<netpp::Channel>)> m_disconnectCb;
	std::function<void(error::SocketError)> m_errorCb;
	std::function<void(signal::Signals)> m_signalCb;

	std::shared_ptr<void> m_impl;
};
}

#endif //NETPP_EVENTS_H
