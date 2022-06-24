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
class AsyncDnsHandler;
class Uri;
using DnsResolvedCallback = std::function<void(Address)>;
enum class ResolveResult {
	E_Timeout,
	E_DnsResolverMissed,
	E_NotFound
};
class DnsResolver {
public:
	DnsResolver();

	void resolve(const std::string &host, const DnsResolvedCallback &callback);
	void resolve(const Uri &uri, const DnsResolvedCallback &callback);

private:
};
}

#endif //NETPP_DNSRESOLVER_H
