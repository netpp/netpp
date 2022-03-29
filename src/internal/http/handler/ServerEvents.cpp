//
// Created by gaojian on 2022/3/29.
//

#include "internal/http/handler/ServerEvents.h"

namespace netpp::internal::http::handler {
void ServerEvents::onConnected([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ServerEvents::onMessageReceived([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ServerEvents::onWriteCompleted([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ServerEvents::onDisconnect([[maybe_unused]] const std::shared_ptr<netpp::Channel> &channel)
{}

void ServerEvents::onError([[maybe_unused]] error::SocketError code)
{}

void ServerEvents::onSignal([[maybe_unused]] signal::Signals signal)
{}
}
