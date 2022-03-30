//
// Created by gaojian on 2022/3/29.
//

#include "internal/handlers/AsyncDnsHandler.h"
#include "EventLoop.h"
#include "time/Timer.h"
#include <cstring>
#include <netdb.h>
#include "Address.h"

namespace {
unsigned toSecond(::timeval* tvp)
{
	return static_cast<unsigned>(tvp->tv_sec) + static_cast<unsigned>(static_cast<double>(tvp->tv_usec) / 1000000.0);
}

struct CAresResolveData {
	netpp::internal::handlers::AsyncDnsHandler *handler;
	netpp::internal::handlers::AsyncDnsHandler::ResolvedCallback cb;
};
}
namespace netpp::internal::handlers {
AsyncDnsHandler::AsyncDnsHandler()
	: m_timeout{nullptr}, m_areasChannel{nullptr}, m_aresSockFd{0}
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
	m_timeout->setSingleShot(true);
	m_timeout->setOnTimeout([this] { resolveTimeout(); });
}

AsyncDnsHandler::~AsyncDnsHandler()
{
	::ares_destroy(m_areasChannel);
}

void AsyncDnsHandler::resolve(const std::string &host, const ResolvedCallback &cb)
{
	auto *data = new CAresResolveData{this, cb};
	::ares_gethostbyname(m_areasChannel, host.c_str(), AF_INET, &AsyncDnsHandler::aresHostResolvedCallback, data);
	::timeval tv{};
	::timeval* tvp = ::ares_timeout(m_areasChannel, nullptr, &tv);
	unsigned timeoutSec = ::toSecond(tvp);
	m_timeout->setInterval(timeoutSec);
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

void AsyncDnsHandler::aresHostResolvedCallback(void* data, int status, [[maybe_unused]] int timeouts, struct ::hostent* hostent)
{
	auto resolveData = static_cast<CAresResolveData *>(data);

	auto address = std::make_shared<::sockaddr_in>();
	std::memset(address.get(), 0, sizeof address);
	address->sin_family = AF_INET;
	address->sin_port = 0;
	if (status)
		address->sin_addr = *reinterpret_cast<in_addr*>(hostent->h_addr);
	Address inet(address);
	resolveData->cb(inet);
}

void AsyncDnsHandler::resolveTimeout()
{
	::ares_process_fd(m_areasChannel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
	::timeval tv{};
	::timeval* tvp = ::ares_timeout(m_areasChannel, nullptr, &tv);
	if (tvp)
	{
		unsigned timeoutSec = ::toSecond(tvp);
		m_timeout->setInterval(timeoutSec);
	}
}
}
