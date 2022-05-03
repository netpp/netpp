//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_ASYNCDNSHANDLER_H
#define NETPP_ASYNCDNSHANDLER_H

#include "internal/epoll/EventHandler.h"
#include "ares.h"
#include <functional>

namespace netpp {
class Address;
namespace time {
class Timer;
}

namespace internal::handlers {
class AsyncDnsHandler : protected epoll::EventHandler, public std::enable_shared_from_this<AsyncDnsHandler> {
public:
	using ResolvedCallback = std::function<void(Address)>;
	explicit AsyncDnsHandler(eventloop::EventLoop *loop);

	~AsyncDnsHandler() override;

	void resolve(const std::string &host, const ResolvedCallback &cb);

protected:
	void handleIn() override;

private:
	static int aresSockCreate(int sock, int type, void *data);
	static void aresSockStateChanged(void *arg, int fd, int readable, int writable);
	static void aresHostResolvedCallback(void* data, int status, int timeouts, struct ::hostent *hostent);

	void resolveTimeout();

private:
	std::unique_ptr<time::Timer> m_timeout;
	::ares_channel m_areasChannel;
	int m_aresSockFd;
};
}
}

#endif //NETPP_ASYNCDNSHANDLER_H
