//
// Created by gaojian on 2022/3/29.
//

#include "internal/http/handler/ClientEvents.h"
#include "internal/http/channel/HttpChannel.h"

namespace netpp::internal::http::handler {
void ClientEvents::onConnected(const std::shared_ptr<netpp::Channel> &channel)
{
	auto httpChannel = std::dynamic_pointer_cast<http::channel::HttpChannel>(channel);

//	httpChannel->addHttpRequest();
}

void ClientEvents::onMessageReceived([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ClientEvents::onWriteCompleted([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ClientEvents::onDisconnect([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ClientEvents::onError([[maybe_unused]] error::SocketError code)
{}

void ClientEvents::onSignal([[maybe_unused]] signal::Signals signal)
{}
}
