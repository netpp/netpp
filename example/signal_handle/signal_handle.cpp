#include "signal/Signals.h"
#include "Application.h"
#include <iostream>

int main()
{
	netpp::Application app;
	app.bindSignalHandler({netpp::Signals::E_QUIT, netpp::Signals::E_ALRM},
						  [](netpp::Signals signal) {
							  std::cout << "received signal " << netpp::signalAsString(signal) << std::endl;
						  });

	app.exec();
}
