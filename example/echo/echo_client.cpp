//
// Created by gaojian on 2020/7/24.
//

#include "TcpClient.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>
#include "Channel.h"
#include "EventLoopDispatcher.h"
#include "support/ThreadPool.hpp"

class Echo {
public:
	Echo() : pool{1} { pool.start(); }

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		// long-term operation is not allowed in event handler
		// it will cause event loop block!!
		pool.run([channel]{
			std::size_t size = channel->availableRead();
			std::string data = channel->retrieveString(size);
			std::cout << "Received size "<< size << " data " << data;
			std::string str;
			std::cin >> str;
			channel->writeString(str);
			channel->send();
		});
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		channel->writeString("hello netpp");
		channel->send();
	}

	void onWriteCompleted()
	{
		std::cout << "Write completed";
	}

private:
	netpp::support::ThreadPool pool;
};

int main()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
	client.connect();
	dispatcher.startLoop();
	return 0;
}
