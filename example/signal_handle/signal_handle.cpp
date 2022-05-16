
#include "signal/Signals.h"
#include "Events.h"
#include "Application.h"
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
	netpp::Config config;
	config.enableLog = true;
	config.enableHandleSignal = true;
	config.eventHandler = netpp::Events(std::make_shared<SignalEvent>());
	netpp::Application app(config);

	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_QUIT);
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_ALRM);

	app.exec();
}
