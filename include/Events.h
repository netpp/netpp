//
// Created by gaojian on 2020/7/7.
//

#ifndef NETPP_EVENTS_H
#define NETPP_EVENTS_H

#include <memory>

namespace netpp {

class Channel;

class Events {
public:
	virtual ~Events() = default;

	virtual void onConnected([[maybe_unused]] std::shared_ptr<netpp::Channel> channel) {};
	virtual void onMessageReceived([[maybe_unused]] std::shared_ptr<netpp::Channel> channel) {};
	virtual void onWriteCompleted() {};
	virtual void onDisconnect() {};
	virtual void onError() {};
	virtual std::unique_ptr<Events> clone() = 0;
};

class DefaultEvents : public Events {
public:
	~DefaultEvents() override = default;
	
	std::unique_ptr<Events> clone() override { return std::make_unique<DefaultEvents>(); }
};
}

#endif //NETPP_EVENTS_H
