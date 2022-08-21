//
// Created by gaojian on 2021/4/13.
//

#include "Application.h"
#include "channel/TcpChannel.h"
#include "Acceptor.h"
#include "error/Error.h"
#include <list>
#include <iostream>

class ChatServer {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		channel->setMessageReceivedCallBack(std::bind(&ChatServer::onMessageReceived, this, std::placeholders::_1));
		channel->setDisconnectedCallBack(std::bind(&ChatServer::onDisconnect, this, std::placeholders::_1));
		channel->setErrorCallBack(std::bind(&ChatServer::onError, this, std::placeholders::_1));

		for (auto &member : m_room)
		{
			if (member != channel)
			{
				std::string hello{"hello!\n"};
				netpp::ByteArray buffer;
				buffer.writeUInt16(static_cast<uint16_t>(hello.length()));
				buffer.writeString(hello);
				member->send(buffer);
			}
		}
		m_room.emplace_back(channel);
	}

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto size = channel->read(sizeof(uint16_t));
		auto msg = channel->read(size.retrieveUInt16());

		auto length = size.retrieveUInt16();
		auto message = msg.retrieveString(length);

		netpp::ByteArray buffer;
		buffer.writeUInt16(length);
		buffer.writeString(message);
		for (auto &member : m_room)
		{
			if (member != channel)
			{
				member->send(buffer);
			}
		}
	}

	void onDisconnect(std::shared_ptr<netpp::Channel> channel)
	{
		netpp::ByteArray buffer;
		std::string goodBye{"good bye!\n"};
		buffer.writeUInt16(static_cast<uint16_t>(goodBye.length()));
		buffer.writeString(goodBye);
		auto it = m_room.begin();
		while (it != m_room.end())
		{
			if ((*it) == channel)
			{
				it = m_room.erase(it);
			}
			else
			{
				(*it)->send(buffer);
				++it;
			}
		}
	}

	void onError(netpp::Error code)
	{
		std::cerr << netpp::getErrorDescription(code) << std::endl;
	}

private:
	std::list<std::shared_ptr<netpp::Channel>> m_room;
};

int main()
{
	netpp::Application app;
	netpp::Acceptor acceptor;
	ChatServer server;
	acceptor.setConnectedCallback(std::bind(&ChatServer::onConnected, &server, std::placeholders::_1));
	acceptor.listen(netpp::Address("0.0.0.0", 12346));
	app.exec();
}
