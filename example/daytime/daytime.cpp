#include "Events.h"
#include "channel/TcpChannel.h"
#include "TcpServer.h"
#include "EventLoopDispatcher.h"
#include <ctime>
#include <iostream>

class DayTime {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		std::time_t time;
		std::time(&time);
		std::string timeStr = std::ctime(&time);
		auto writer = tcpChannel->writer();
		writer.writeString(timeStr + "\r\n");
		channel->send();
		channel->close();
	}

	void onWriteCompleted([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		std::cout << "Write completed";
	}

	void onDisconnect([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		std::cout << "Disconnected";
	}
};

int main()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events dayTime(std::make_shared<DayTime>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(dayTime));
	server.listen();
	dispatcher.startLoop();
}
