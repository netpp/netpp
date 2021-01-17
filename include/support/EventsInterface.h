#ifndef NETPP_EVENTS_INTERFACE_H
#define NETPP_EVENTS_INTERFACE_H

#include "signal/Signals.h"
#include <memory>

#define ASSERT_HAS_EVENT_METHOD(METHOD, ARG...)		\
template<typename T>	\
class has##METHOD {					\
public:								\
	template<typename U, void(U::*)(ARG) = &U::on##METHOD>			\
	static constexpr bool assert_has(U*) { return true; }			\
	static constexpr bool assert_has(...) { return false; }			\
	static constexpr bool value = assert_has(static_cast<T*>(0));	\
};

namespace netpp {
class Channel;

namespace support {
/**
 * @brief The Events interface for internal use
 * do NOT inherts this!!
 */
class EventInterface {
public:
	virtual ~EventInterface() = default;
	virtual void onConnected(std::shared_ptr<netpp::Channel> channel) = 0;
	virtual void onMessageReceived(std::shared_ptr<netpp::Channel> channel) = 0;
	virtual void onWriteCompleted() = 0;
	virtual void onDisconnect() = 0;
	virtual void onError() = 0;
	virtual void onSignal(signal::Signals signal) = 0;
	virtual std::unique_ptr<support::EventInterface> clone() = 0;
};

// TODO: move this into Events.cpp to optimize compile time
ASSERT_HAS_EVENT_METHOD(Connected, std::shared_ptr<netpp::Channel>)
ASSERT_HAS_EVENT_METHOD(MessageReceived, std::shared_ptr<netpp::Channel>)
ASSERT_HAS_EVENT_METHOD(WriteCompleted)
ASSERT_HAS_EVENT_METHOD(Disconnect)
ASSERT_HAS_EVENT_METHOD(Error)
ASSERT_HAS_EVENT_METHOD(Signal, signal::Signals)

}
}
#endif
