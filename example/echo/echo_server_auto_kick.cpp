#include "Acceptor.h"
#include "channel/TcpChannel.h"
#include "Application.h"
#include <iostream>

int main()
{
	netpp::Application app;
	netpp::Acceptor acceptor;
	acceptor.setConnectedCallback([](std::shared_ptr<netpp::Channel> channel){
		channel->setIdleTimeout(8000);
		auto size = channel->readableBytes();
		std::string data = channel->read(size).retrieveString(size);
		std::cout << "received " << data << " on channel" << std::endl;
		netpp::ByteArray buffer;
		buffer.writeString(data);
		channel->send(buffer);
	});
	acceptor.listen(netpp::Address("0.0.0.0", 12347));
	app.exec();
}
