#include "TcpServer.h"
#include "Events.h"
#include "channel/Channel.h"
#include "EventLoopDispatcher.h"
#include <iostream>

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto reader = channel->reader();
		std::size_t size = reader.readableBytes();
		std::string data = reader.retrieveString(size).value();
		std::cout << "received " << data << " on channel" << std::endl;
		auto writer = channel->writer();
		writer.writeString(data);
		channel->send();
	}
};

int main()
{
	netpp::EventLoopDispatcher dispatcher(1, 1000, 8);
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
	server.listen();
	dispatcher.startLoop();
}
