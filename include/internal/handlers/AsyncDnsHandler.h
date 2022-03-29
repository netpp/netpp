//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_ASYNCDNSHANDLER_H
#define NETPP_ASYNCDNSHANDLER_H

#include "internal/epoll/EventHandler.h"
#include "ares.h"

namespace netpp::dns {
class DnsResolver;
}

namespace netpp::internal::handlers {
class AsyncDnsHandler : public epoll::EventHandler, public std::enable_shared_from_this<AsyncDnsHandler>  {
public:
	explicit AsyncDnsHandler(dns::DnsResolver *resolver);
	~AsyncDnsHandler() override;

protected:
	void handleIn() override;

private:
	static int aresSockCreate(int sock, int type, void* data);
	static void aresSockStateChanged(void *arg, int fd, int readable, int writable);

private:
	dns::DnsResolver *_resolver;
	::ares_channel m_areasChannel;
	int m_aresSockFd;
};
}

#endif //NETPP_ASYNCDNSHANDLER_H
