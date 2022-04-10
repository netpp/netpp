#include "time/Timer.h"
#include <iostream>
#include "Application.h"

int main()
{
	netpp::Config config;
//	config.tickTimer.enable = false;
	netpp::Application app(config);
	netpp::time::Timer timer;
	timer.setInterval(1000);
	timer.setSingleShot(false);
	timer.setOnTimeout([&timer]{
		static int timerTriggerCount = 0;
		std::cout << "Timer triggered count " << timerTriggerCount << std::endl;
		++timerTriggerCount;
		if (timerTriggerCount > 5)
			timer.stop();
	});
	timer.start();
	app.exec();
}
