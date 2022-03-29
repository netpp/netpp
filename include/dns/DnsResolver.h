//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_DNSRESOLVER_H
#define NETPP_DNSRESOLVER_H

#include <string>
#include <functional>

namespace netpp {
namespace uri {
class Uri;
}
namespace dns {
using DnsResolvedCallback = std::function<void()>;
class DnsResolver {
public:
	DnsResolver();

	void resolve(const std::string &host, DnsResolvedCallback callback);
	void resolve(const uri::Uri &uri, DnsResolvedCallback callback);
};
}
}

#endif //NETPP_DNSRESOLVER_H
