#include "signal/SignalWatcher.h"
#include "handlers/SignalHandler.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>
#include "Log.h"

class SignalEvent : public netpp::Events {
public:
	~SignalEvent() override = default;
	void onSignal(netpp::signal::Signals signal)
	{
		std::cout << "received signal " << netpp::signal::signalAsString(signal) << std::endl;
	}
	std::unique_ptr<Events> clone() override { return std::make_unique<SignalEvent>(); }
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();
	netpp::handlers::SignalHandler::makeSignalHandler(loop, std::make_unique<SignalEvent>());
	netpp::signal::SignalWatcher watcher;
	watcher.watch(netpp::signal::Signals::E_QUIT);
	dispatcher.startLoop();
}
