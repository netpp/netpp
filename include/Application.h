//
// Created by gaojian on 2022/3/31.
//

#ifndef NETPP_APPLICATION_H
#define NETPP_APPLICATION_H

#include <memory>
#include "support/Types.h"

namespace netpp {
class EventLoopManager;
/**
 * @brief An Application take care of global resources in netpp, such as event loops, there should be only on instance of Application.
 */
class Application {
public:
	/**
	 * @brief Create an application, and init global resources
	 * @param config The m_config to init resource
	 */
	explicit Application();
	~Application();

	/**
	 * @brief Start all event loops
	 */
	void exec();

	void bindSignalHandler(std::initializer_list<Signals> interestedSignals, SignalCallBack cb);

	static Application *instance();
	static EventLoopManager *loopManager();

private:
	std::unique_ptr<EventLoopManager> m_loopManager;
};
}

#endif //NETPP_APPLICATION_H
