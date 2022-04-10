//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_DNSRESOLVER_H
#define NETPP_DNSRESOLVER_H

#include <string>
#include <functional>
#include <memory>

namespace netpp {
class Address;
namespace uri {
class Uri;
}
namespace internal::handlers {
class AsyncDnsHandler;
}
namespace dns {
using DnsResolvedCallback = std::function<void(Address)>;
class DnsResolver {
public:
	DnsResolver();

	void resolve(const std::string &host, const DnsResolvedCallback &callback);
	void resolve(const uri::Uri &uri, const DnsResolvedCallback &callback);

private:
	std::shared_ptr<internal::handlers::AsyncDnsHandler> m_asyncDnsHandler;
};
}
}

#endif //NETPP_DNSRESOLVER_H
