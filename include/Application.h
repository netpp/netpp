//
// Created by gaojian on 2022/3/31.
//

#ifndef NETPP_APPLICATION_H
#define NETPP_APPLICATION_H

#include "Config.h"
#include "eventloop/EventLoopManager.h"

namespace netpp {
/**
 * @brief An Application take care of global resources in netpp, such as event loops, there should be only on instance of Application.
 */
class Application {
public:
	/**
	 * @brief Create an application, and init global resources
	 * @param config The m_config to init resource
	 */
	explicit Application(Config config = {});
	~Application();

	/**
	 * @brief Start all event loops
	 */
	void exec();

	static Application *instance();
	static eventloop::EventLoopManager *loopManager();
	static Config appConfig();

private:
	Config m_netppConfig;
	std::unique_ptr<eventloop::EventLoopManager> m_loopManager;
};
}

#endif //NETPP_APPLICATION_H
