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
	std::unique_ptr<netpp::Events<DayTime>> dayTime = std::make_unique<netpp::Events<DayTime>>(DayTime());
	netpp::TcpServer server(&dispatcher, std::move(dayTime));
	server.listen((netpp::Address("0.0.0.0", 12345)));
	dispatcher.startLoop();
}
