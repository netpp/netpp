//
// Created by gaojian on 2020/7/7.
//

#ifndef NETPP_EVENTS_H
#define NETPP_EVENTS_H

#include <functional>
#include "Channel.h"
#include <memory>
#include "error/SocketError.h"
#include "signal/Signals.h"

namespace netpp {

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

/**
 * @brief netpp events, callbacks
 * 
 */
class Events final {
public:
	Events() : m_impl{nullptr} {}

	/**
	 * @brief Construct a new Events object
	 * 
	 * @tparam Impl		The user event handler
	 * @param impl		shared_ptr to event handler, all event loop will shared the same instance, make sure it's thread safe
	 */
	template<typename Impl>
	explicit Events(std::shared_ptr<Impl> impl)
	: m_impl{impl}
	{
		Impl *implPtr = impl.get();
		if constexpr (hasConnected<Impl>::value)
			m_connectedCb = std::bind(&Impl::onConnected, implPtr, std::placeholders::_1);
		if constexpr (hasMessageReceived<Impl>::value)
			m_receiveMsgCb = std::bind(&Impl::onMessageReceived, implPtr, std::placeholders::_1);
		if constexpr (hasWriteCompleted<Impl>::value)
			m_writeCompletedCb = std::bind(&Impl::onWriteCompleted, implPtr);
		if constexpr (hasDisconnect<Impl>::value)
			m_disconnectCb = std::bind(&Impl::onDisconnect, implPtr);
		if constexpr (hasError<Impl>::value)
			m_errorCb = std::bind(&Impl::onError, implPtr, std::placeholders::_1);
		if constexpr (hasSignal<Impl>::value)
			m_signalCb = std::bind(&Impl::onSignal, implPtr, std::placeholders::_1);
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{ if (m_connectedCb) m_connectedCb(channel); }

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{ if (m_receiveMsgCb) m_receiveMsgCb(channel); }

	void onWriteCompleted()
	{ if (m_writeCompletedCb) m_writeCompletedCb(); }

	void onDisconnect()
	{ if (m_disconnectCb) m_disconnectCb(); }

	void onError(error::SocketError code)
	{ if (m_errorCb) m_errorCb(code); }

	void onSignal(signal::Signals signal)
	{ if (m_signalCb) m_signalCb(signal); }

private:
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
