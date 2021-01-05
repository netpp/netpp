//
// Created by gaojian on 2020/7/6.
//

#ifndef NETPP_TCPSERVER_H
#define NETPP_TCPSERVER_H

#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
class Channel;

class TcpServer {
public:
	explicit TcpServer(EventLoopDispatcher *dispatcher, std::unique_ptr<Events> &&eventsPrototype);

	void listen(Address listenAddr);

private:
	Channel *_acceptorChannel;
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<Events> _eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
