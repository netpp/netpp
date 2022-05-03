//
// Created by gaojian on 2022/3/31.
//

#include "Application.h"
#include <utility>
#include "eventloop/EventLoop.h"
#include "internal/support/Log.h"
#include "eventloop/EventLoopManager.h"
#include "signal/SignalWatcher.h"
#include <cassert>

namespace {
netpp::Application *globalApp = nullptr;
}

namespace netpp {
Application::Application(Config config)
	: m_netppConfig{std::move(config)}
{
	assert(!globalApp);
	::globalApp = this;
	if (m_netppConfig.enableLog)
		internal::initLogger();
	if (m_netppConfig.enableHandleSignal)
		signal::SignalWatcher::enableWatchSignal();
	m_loopManager = std::make_unique<eventloop::EventLoopManager>(m_netppConfig);
}

Application::~Application()
{
	::globalApp = nullptr;
}

void Application::exec()
{
	m_loopManager->startLoop();
}

Application *Application::instance()
{
	return ::globalApp;
}

eventloop::EventLoopManager *Application::loopManager()
{
	return ::globalApp->m_loopManager.get();
}

Config Application::appConfig()
{
	return ::globalApp->m_netppConfig;
}
}
