//
// Created by gaojian on 2021/4/13.
//

#include "Events.h"
#include "TcpServer.h"
#include "EventLoopDispatcher.h"
#include "Channel.h"
#include <list>
#include <iostream>
#include "error/SocketError.h"

class ChatServer {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		std::lock_guard lck(mutex);
		for (auto &member : m_room)
		{
			if (member != channel)
			{
				std::string hello{"hello!\n"};
				member->writeUInt16(static_cast<uint16_t>(hello.length()));
				member->writeString(hello);
				member->send();
			}
		}
		m_room.emplace_back(channel);
	}

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		uint16_t length = channel->retrieveUInt16();
		std::string message = channel->retrieveString(length);
		std::lock_guard lck(mutex);
		for (auto &member : m_room)
		{
			if (member->channelId() != channel->channelId())
			{
				member->writeUInt16(length);
				member->writeString(message);
				member->send();
			}
		}
	}

	void onDisconnect(std::shared_ptr<netpp::Channel> channel)
	{
		std::lock_guard lck(mutex);
		auto it = m_room.begin();
		while (it != m_room.end())
		{
			if ((*it)->channelId() == channel->channelId())
			{
				it = m_room.erase(it);
			}
			else
			{
				std::string hello{"good bye!\n"};
				std::shared_ptr<netpp::Channel> &onlineChannel = (*it);
				onlineChannel->writeUInt16(static_cast<uint16_t>(hello.length()));
				onlineChannel->writeString(hello);
				++it;
			}
		}
	}

	void onError(netpp::error::SocketError code)
	{
		std::cerr << netpp::error::errorAsString(code) << std::endl;
	}

private:
	std::mutex mutex;
	std::list<std::shared_ptr<netpp::Channel>> m_room;
};

int main()
{
	netpp::EventLoopDispatcher dispatcher(2);
	netpp::TcpServer chatServer(&dispatcher, netpp::Address(), netpp::Events(std::make_shared<ChatServer>()));
	chatServer.listen();
	dispatcher.startLoop();
}
