#include "time/Timer.h"
#include "EventLoop.h"
#include <iostream>

int main()
{
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(1000);
		timer->setSingleShot(false);
		timer->setOnTimeout([]{
			static int timerTriggerCount = 0;
			std::cout << "Timer triggered count " << timerTriggerCount << std::endl;
			++timerTriggerCount;
		});
		timer->start();
	});
	loop.run();
}
