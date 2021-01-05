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

class Echo : public netpp::Events {
public:
	~Echo() override = default;

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel) override
	{
		std::size_t size = channel->availableRead();
		std::string data = channel->retrieveString(size);
		SPDLOG_LOGGER_TRACE(netpp::logger, "Received size {} data {}", size, data);
		std::string str;
		std::cin >> str;
		channel->writeString(str);
		channel->send();
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel) override
	{
		channel->writeString("hello netpp");
		channel->send();
	}

	void onWriteCompleted() override
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Write completed");
	}

	std::unique_ptr<netpp::Events> clone() override
	{
		return std::make_unique<Echo>();
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::TcpClient client(&dispatcher, std::make_unique<Echo>());
	client.connect(netpp::Address("127.0.0.1", 12345));
	dispatcher.startLoop();
	return 0;
}
