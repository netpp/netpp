//
// Created by gaojian on 22-5-17.
//

#include <gtest/gtest.h>
#include "channel/Channel.h"
#include "Application.h"
#include "TcpServer.h"
#include "TcpClient.h"

class ConnectionTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

/*class EventHandler {
public:
	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{}

	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{}

	void onWriteCompleted(std::shared_ptr<netpp::Channel> channel)
	{}

	void onDisconnect(std::shared_ptr<netpp::Channel> channel)
	{}
};*/

TEST_F(ConnectionTest, CallbackTest)
{
	/*netpp::Config config;
	config.eventHandler = netpp::Events(std::make_shared<EventHandler>());
	netpp::Application app(config);

	netpp::TcpServer server(netpp::Address("0.0.0.0", 4321));
	server.listen();

	netpp::TcpClient client(netpp::Address("0.0.0.0", 4321));
	client.connect();

	app.exec();*/
}

TEST_F(ConnectionTest, EchoTest)
{}
