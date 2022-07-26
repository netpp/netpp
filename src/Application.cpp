//
// Created by gaojian on 2022/3/31.
//

#include "Application.h"
#include <utility>
#include "eventloop/EventLoop.h"
#include "support/Log.h"
#include "eventloop/EventLoopManager.h"
#include <cassert>
#include "epoll/handlers/SignalHandler.h"

namespace {
netpp::Application *globalApp = nullptr;
}

namespace netpp {
Application::Application()
{
	assert(!globalApp);
	::globalApp = this;
	initLogger();
	m_loopManager = std::make_unique<EventLoopManager>(std::thread::hardware_concurrency());
}

Application::~Application()
{
	::globalApp = nullptr;
}

void Application::exec()
{
	m_loopManager->startLoop();
}

void Application::bindSignalHandler(std::initializer_list<Signals> interestedSignals, SignalCallBack cb)
{
	auto mainLoop = m_loopManager->mainLoop();

	auto signalHandler = std::make_shared<SignalHandler>(mainLoop, interestedSignals);
	signalHandler->setSignalCallback(std::move(cb));
	mainLoop->addEventHandlerToLoop(signalHandler);
}

Application *Application::instance()
{
	return ::globalApp;
}

EventLoopManager *Application::loopManager()
{
	return ::globalApp->m_loopManager.get();
}
}
