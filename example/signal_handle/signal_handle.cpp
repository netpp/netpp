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
	netpp::signal::SignalWatcher::enableWatchSignal();

	netpp::EventLoopDispatcher dispatcher;
	netpp::Events event(std::make_shared<SignalEvent>());
	netpp::signal::SignalWatcher::with(&dispatcher, std::move(event))
								.watch(netpp::signal::Signals::E_QUIT)
								.watch(netpp::signal::Signals::E_ALRM);
	dispatcher.startLoop();
}
