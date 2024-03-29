//
// Created by gaojian on 2021/4/13.
//

#include "Events.h"
#include "TcpServer.h"
#include "EventLoopDispatcher.h"
#include "channel/TcpChannel.h"
#include <list>
#include <iostream>
#include "error/SocketError.h"

class ChatServer {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		for (auto &member : m_room)
		{
			if (member != channel)
			{
				auto tcpMemberChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(member);
				auto writer = tcpMemberChannel->writer();
				std::string hello{"hello!\n"};
				writer.writeUInt16(static_cast<uint16_t>(hello.length())).writeString(hello);
				member->send();
			}
		}
		m_room.emplace_back(channel);
	}

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		auto reader = tcpChannel->reader();
		uint16_t length = reader.retrieveUInt16().value();
		std::string message = reader.retrieveString(length).value();
		for (auto &member : m_room)
		{
			if (member->channelId() != channel->channelId())
			{
				auto tcpMemberChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(member);
				auto writer = tcpMemberChannel->writer();
				writer.writeUInt16(length).writeString(message);
				member->send();
			}
		}
	}

	void onDisconnect(std::shared_ptr<netpp::Channel> channel)
	{
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
				auto tcpOnlineChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(*it);
				auto writer = tcpOnlineChannel->writer();
				writer.writeUInt16(static_cast<uint16_t>(hello.length())).writeString(hello);
				tcpOnlineChannel->send();
				++it;
			}
		}
	}

	void onError(netpp::error::SocketError code)
	{
		std::cerr << netpp::error::errorAsString(code) << std::endl;
	}

private:
	std::list<std::shared_ptr<netpp::Channel>> m_room;
};

int main()
{
	netpp::EventLoopDispatcher dispatcher(2);
	netpp::TcpServer chatServer(&dispatcher, netpp::Address(), netpp::Events(std::make_shared<ChatServer>()));
	chatServer.listen();
	dispatcher.startLoop();
}
