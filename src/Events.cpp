//
// Created by gaojian on 2021/3/20.
//

#include "Events.h"
#include "support/ThreadPool.hpp"

namespace netpp {
void Events::onConnected(const std::shared_ptr<netpp::Channel> &channel)
{
	if (m_connectedCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_connectedCb, channel);
		else
			m_connectedCb(channel);
	}
}

void Events::onMessageReceived(const std::shared_ptr<netpp::Channel> &channel)
{
	if (m_receiveMsgCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_receiveMsgCb, channel);
		else
			m_receiveMsgCb(channel);
	}
}

void Events::onWriteCompleted()
{
	if (m_writeCompletedCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_writeCompletedCb);
		else
			m_writeCompletedCb();
	}
}

void Events::onDisconnect()
{
	if (m_disconnectCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_disconnectCb);
		else
			m_disconnectCb();
	}
}

void Events::onError(error::SocketError code)
{
	if (m_errorCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_errorCb, code);
		else
			m_errorCb(code);
	}
}

void Events::onSignal(signal::Signals signal)
{
	if (m_signalCb)
	{
		if (m_eventsPool)
			m_eventsPool->run(m_signalCb, signal);
		else
			m_signalCb(signal);
	}
}


void Events::initThread(int threadsCount)
{
	if (threadsCount > 0)	// if thread number is less than 0, not run handler in thread
	{
		m_eventsPool = std::make_unique<support::ThreadPool>(threadsCount);
		// FIXME: start threads after runs loop, start too soon may cause block signal fail
		m_eventsPool->start();
	}
}
}