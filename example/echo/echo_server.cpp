//
// Created by gaojian on 2020/7/5.
//

#include "TcpServer.h"
#include "Events.h"
#include "Channel.h"
#include "EventLoopDispatcher.h"
#include <iostream>

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		std::size_t size = channel->availableRead();
		std::string data = channel->retrieveString(size);
		std::cout << "received " << data << " on channel";
		channel->writeString(data);
		channel->send();
	}
};

int main()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
	server.listen();
	dispatcher.startLoop();
}
