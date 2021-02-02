#include "signal/SignalWatcher.h"
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
	std::unique_ptr<netpp::Events<SignalEvent>> event = std::make_unique<netpp::Events<SignalEvent>>();
	netpp::signal::SignalWatcher::enableWatchSignal(dispatcher.dispatchEventLoop(), std::move(event));
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_QUIT);
	dispatcher.startLoop();
}
