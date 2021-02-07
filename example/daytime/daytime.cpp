#include "Events.h"
#include "Channel.h"
#include "TcpServer.h"
#include "Log.h"
#include "EventLoopDispatcher.h"
#include <ctime>

class DayTime {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		std::time_t time;
		std::time(&time);
		std::string timeStr = std::ctime(&time);
		channel->writeString(timeStr + "\r\n");
		channel->send();
		channel->close();
	}

	void onWriteCompleted()
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Write completed");
	}

	void onDisconnect()
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Disconnected");
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events dayTime(std::make_shared<DayTime>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(dayTime));
	server.listen();
	dispatcher.startLoop();
}
