#include "time/Timer.h"
#include "time/TimeWheel.h"
#include "EventLoop.h"
#include <iostream>

class WheelEntry : public netpp::time::TimeWheelEntry {
public:
	~WheelEntry() override = default;
	void onTimeout() override
	{ std::cout << "wheel entry timeout" << std::endl; }
};

int main()
{
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	std::shared_ptr<netpp::time::TimeWheel> wheel;
	loop.runInLoop([&]{
		wheel = std::make_shared<netpp::time::TimeWheel>(&loop, 1000, 3);
		wheel->addToWheel(std::make_shared<WheelEntry>());

		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(1000);
		timer->setSingleShot(false);
		timer->setOnTimeout([wheel]{
			static int timerTriggerCount = 0;
			std::cout << "Timer triggered count" << timerTriggerCount << std::endl;
			++timerTriggerCount;
			if (timerTriggerCount % 3 == 0)
				wheel->addToWheel(std::make_shared<WheelEntry>());
		});
		timer->start();
	});
	loop.run();
}
