//
// Created by gaojian on 2020/7/5.
//

#include "channel/TcpChannel.h"
#include "Application.h"
#include "Acceptor.h"
#include <iostream>

int main()
{
	netpp::Application app;
	netpp::Acceptor acceptor;
	acceptor.setConnectedCallback([](std::shared_ptr<netpp::Channel> channel){
		channel->setMessageReceivedCallBack([](std::shared_ptr<netpp::Channel> channel){
			auto size = channel->readableBytes();
			auto dataByteArray = channel->read(size);
			std::string data = dataByteArray.retrieveString(size);
			std::cout << "received " << data << " on channel";

			netpp::ByteArray buffer;
			buffer.writeString(data);
			channel->send(buffer);
		});
	});
	acceptor.listen(netpp::Address("0.0.0.0", 12347));
	app.exec();
}
