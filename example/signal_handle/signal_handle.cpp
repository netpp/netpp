#include "signal/SignalWatcher.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>

class SignalEvent {
public:
	void onSignal(netpp::signal::Signals signal)
	{
		std::cout << "received signal " << netpp::signal::signalAsString(signal) << std::endl;
	}
};

int main()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events event(std::make_shared<SignalEvent>());
	netpp::signal::SignalWatcher::enableWatchSignal(&dispatcher, std::move(event));
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_QUIT);
	dispatcher.startLoop();
}
