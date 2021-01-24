#include "signal/SignalWatcher.h"
#include "handlers/SignalHandler.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>
#include "Log.h"

class SignalEvent {
public:
	void onSignal(netpp::signal::Signals signal)
	{
		std::cout << "received signal " << netpp::signal::signalAsString(signal) << std::endl;
	}
};

int main()
{
	netpp::initLogger();
	netpp::EventLoopDispatcher dispatcher;
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();
	std::unique_ptr<netpp::Events<SignalEvent>> event = std::make_unique<netpp::Events<SignalEvent>>(SignalEvent());
	netpp::handlers::SignalHandler::makeSignalHandler(loop, std::move(event));
	netpp::signal::SignalWatcher watcher;
	watcher.watch(netpp::signal::Signals::E_QUIT);
	dispatcher.startLoop();
}
