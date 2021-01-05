//
// Created by gaojian on 2020/7/5.
//

#include "TcpServer.h"
#include "Log.h"
#include "Events.h"
#include "Channel.h"
#include "EventLoopDispatcher.h"

class Echo : public netpp::Events {
public:
	~Echo() override = default;

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel) override
	{
		std::size_t size = channel->availableRead();
		std::string data = channel->retrieveString(size);
		SPDLOG_LOGGER_TRACE(netpp::logger, "received {} on channel", data);
		channel->writeString(data);
		channel->send();
	}

	std::unique_ptr<netpp::Events> clone() override { return std::make_unique<Echo>(); }
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::TcpServer server(&dispatcher, std::make_unique<Echo>());
	server.listen((netpp::Address("0.0.0.0", 12345)));
	dispatcher.startLoop();
}
