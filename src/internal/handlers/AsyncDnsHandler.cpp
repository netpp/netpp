//
// Created by gaojian on 2022/3/29.
//

#include "internal/handlers/AsyncDnsHandler.h"
#include "EventLoop.h"

namespace netpp::internal::handlers {
AsyncDnsHandler::AsyncDnsHandler(dns::DnsResolver *resolver)
	: _resolver{resolver}, m_areasChannel{nullptr}
{
	::ares_options options{};
	int optionMask = ARES_OPT_FLAGS;
	options.flags = ARES_FLAG_NOCHECKRESP;
	options.flags |= ARES_FLAG_STAYOPEN;
	options.flags |= ARES_FLAG_IGNTC; // UDP only
	optionMask |= ARES_OPT_SOCK_STATE_CB;
	options.sock_state_cb = &AsyncDnsHandler::aresSockStateChanged;
	options.sock_state_cb_data = this;
	optionMask |= ARES_OPT_TIMEOUT;
	options.timeout = 2;
	/*if (opt == kDNSonly)
	{
		optionMask |= ARES_OPT_LOOKUPS;
		options.lookups = lookups;
	}*/

	if ((::ares_init_options(&m_areasChannel, &options, optionMask)) != ARES_SUCCESS) {
	}
	::ares_set_socket_callback(m_areasChannel, &AsyncDnsHandler::aresSockCreate, this);
}

AsyncDnsHandler::~AsyncDnsHandler()
{
	::ares_destroy(m_areasChannel);
}

void AsyncDnsHandler::handleIn()
{
	::ares_process_fd(m_areasChannel, m_aresSockFd, ARES_SOCKET_BAD);
}

int AsyncDnsHandler::aresSockCreate(int sock, [[maybe_unused]] int type, void* data)
{
	auto dnsHandler = static_cast<AsyncDnsHandler *>(data);
	dnsHandler->m_aresSockFd = sock;
	dnsHandler->m_epollEvent = make_unique<internal::epoll::EpollEvent>(dnsHandler->_loopThisHandlerLiveIn->getPoll(), dnsHandler->weak_from_this(), sock);
	dnsHandler->m_epollEvent->active(internal::epoll::EpollEv::IN);
	return 0;
}

void AsyncDnsHandler::aresSockStateChanged([[maybe_unused]] void *arg, [[maybe_unused]] int fd, [[maybe_unused]] int readable, [[maybe_unused]] int writable)
{
}
}
