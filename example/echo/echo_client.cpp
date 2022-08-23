//
// Created by gaojian on 2020/7/24.
//

#include "Connector.h"
#include <iostream>
#include "channel/TcpChannel.h"
#include "Application.h"
#include "location/Address.h"
#include "buffer/ByteArray.h"

int main()
{
	netpp::Application app;
	netpp::Connector connector;
	connector.setConnectedCallBack([](std::shared_ptr<netpp::Channel> channel)
	{
		channel->setMessageReceivedCallBack([](std::shared_ptr<netpp::Channel> channel)
		{
			auto size = channel->readableBytes();
			auto dataByteArray = channel->read(size);
			std::string data = dataByteArray.retrieveString(size);

			std::cout << "Received size "<< size << " data " << data;
			std::string str;
			std::cin >> str;
			netpp::ByteArray buffer;
			buffer.writeString(str);
			channel->send(buffer);
		});
		channel->setDisconnectedCallBack([](std::shared_ptr<netpp::Channel> channel){
			std::cout << "Disconnected from server, exit" << channel;
			::exit(0);
		});
		channel->setWriteCompletedCallBack([](std::shared_ptr<netpp::Channel> channel){
			std::cout << "Write completed" << channel;
		});

		netpp::ByteArray buffer;
		std::string greeting("hello netpp");
		buffer.writeString(greeting);
		channel->send(buffer);
	});
	connector.connect(netpp::Address("127.0.0.1", 12347));
	app.exec();
}
