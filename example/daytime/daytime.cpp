#include "Events.h"
#include "Channel.h"
#include "TcpServer.h"
#include "Log.h"
#include "EventLoopDispatcher.h"
#include <ctime>

class DayTime : public netpp::Events {
public:
	~DayTime() override = default;

	void onConnected(std::shared_ptr<netpp::Channel> channel) override
	{
		std::time_t time;
		std::time(&time);
		std::string timeStr = std::ctime(&time);
		channel->writeString(timeStr + "\r\n");
		channel->send();
		channel->close();
	}

	void onWriteCompleted() override
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Write completed");
	}

	void onDisconnect() override
	{
		SPDLOG_LOGGER_TRACE(netpp::logger, "Disconnected");
	}

	std::unique_ptr<netpp::Events> clone() override
	{
		return std::make_unique<DayTime>();
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::TcpServer server(&dispatcher, std::make_unique<DayTime>());
	server.listen((netpp::Address("0.0.0.0", 12345)));
	dispatcher.startLoop();
}
