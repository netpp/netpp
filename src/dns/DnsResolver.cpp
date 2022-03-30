//
// Created by gaojian on 2022/3/29.
//

#include "dns/DnsResolver.h"
#include "uri/Uri.h"
#include "internal/handlers/AsyncDnsHandler.h"

namespace netpp::dns {

DnsResolver::DnsResolver()
{}

void DnsResolver::resolve(const std::string &host, const DnsResolvedCallback &callback)
{
	if (m_asyncDnsHandler)
		m_asyncDnsHandler->resolve(host, callback);
}

void DnsResolver::resolve(const uri::Uri &uri, const DnsResolvedCallback &callback)
{
	resolve(uri.host(), callback);
}
}