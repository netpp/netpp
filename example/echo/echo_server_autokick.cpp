#include "TcpServer.h"
#include "Log.h"
#include "Events.h"
#include "Channel.h"
#include "EventLoopDispatcher.h"

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		std::size_t size = channel->availableRead();
		std::string data = channel->retrieveString(size);
		SPDLOG_LOGGER_TRACE(netpp::logger, "received {} on channel", data);
		channel->writeString(data);
		channel->send();
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher(1, 1000, 8);
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
	server.listen();
	dispatcher.startLoop();
}
