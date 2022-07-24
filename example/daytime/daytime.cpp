#include "channel/TcpChannel.h"
#include "Application.h"
#include "Acceptor.h"
#include "buffer/ByteArray.h"
#include <ctime>
#include <iostream>

int main()
{
	netpp::Application app;
	netpp::Acceptor acceptor;
	acceptor.setConnectedCallback([](std::shared_ptr<netpp::Channel> channel)
								  {
									  std::cout << "Connected" << std::endl;
									  auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
									  tcpChannel->setWriteCompletedCallBack([]([[maybe_unused]] std::shared_ptr<netpp::Channel> channel){
										  std::cout << "Write completed" << std::endl;
									  });
									  tcpChannel->setDisconnectedCallBack([]([[maybe_unused]] std::shared_ptr<netpp::Channel> channel){
										  std::cout << "Disconnected" << std::endl;
									  });
									  std::time_t time;
									  std::time(&time);
									  std::string timeStr = std::ctime(&time);
									  netpp::ByteArray data;
									  data.writeString(timeStr + "\r\n");
									  tcpChannel->send(data);
									  channel->close();
								  });
	acceptor.listen(netpp::Address("127.0.0.1", 12345));
	app.exec();
}
