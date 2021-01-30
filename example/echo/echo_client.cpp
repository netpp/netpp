//
// Created by gaojian on 2020/7/24.
//

#include "TcpClient.h"
#include "Log.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>
#include "Channel.h"
#include "EventLoopDispatcher.h"

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		std::size_t size = channel->availableRead();
		std::string data = channel->retrieveString(size);
		SPDLOG_LOGGER_TRACE(netpp::logger, "Received size {} data {}", size, data);
		std::string str;
		// long-term operation is not allowed in event handler
		// it will cause event loop block!!
		std::cin >> str;
		channel->writeString(str);
		channel->send();
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		channel->writeString("hello netpp");
		channel->send();
	}

	void onWriteCompleted()
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Write completed");
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	std::unique_ptr<netpp::Events<Echo>> events = std::make_unique<netpp::Events<Echo>>(Echo());
	netpp::TcpClient client(&dispatcher, std::move(events));
	client.connect(netpp::Address("127.0.0.1", 12345));
	dispatcher.startLoop();
	return 0;
}
