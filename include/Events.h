//
// Created by gaojian on 2020/7/7.
//

#ifndef NETPP_EVENTS_H
#define NETPP_EVENTS_H

#include "support/EventsInterface.h"

namespace netpp {
template<typename Impl>
class Events final : public support::EventInterface {
public:
	Events() : m_impl() {}
	explicit Events(Impl impl) : m_impl{impl} {}

	void onConnected(std::shared_ptr<netpp::Channel> channel) override
	{ if constexpr (support::hasConnected<Impl>::value) m_impl.onConnected(channel); }

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel) override
	{ if constexpr (support::hasMessageReceived<Impl>::value) m_impl.onMessageReceived(channel); }

	void onWriteCompleted() override
	{ if constexpr (support::hasWriteCompleted<Impl>::value) m_impl.onWriteCompleted(); }

	void onDisconnect() override
	{ if constexpr (support::hasDisconnect<Impl>::value) m_impl.onDisconnect(); }

	void onError(error::SocketError code) override
	{ if constexpr (support::hasError<Impl>::value) m_impl.onError(code); }

	void onSignal(signal::Signals signal) override
	{ if constexpr (support::hasSignal<Impl>::value) m_impl.onSignal(signal); }
	
	std::unique_ptr<support::EventInterface> clone() override { return std::make_unique<Events<Impl>>(m_impl); }

private:
	Impl m_impl;
};
}

#endif //NETPP_EVENTS_H
