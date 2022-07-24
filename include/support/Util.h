//
// Created by gaojian on 22-6-16.
//

#ifndef NETPP_UTIL_H
#define NETPP_UTIL_H

#include "Types.h"

struct sockaddr_in;

namespace netpp {
class Address;

inline const utf8string& toUtf8String(const utf8string& val) { return val; }

/**
 * @brief Convert to utf8 string
 * @tparam Source The type of value
 * @param val the value to be convert
 * @return utf8 string
 */
template<typename Source>
utf8string toUtf8String(const Source& val)
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic());
	oss << val;
	if (oss.bad())
	{
		throw std::bad_cast();
	}
	return oss.str();
}

extern ::sockaddr_in toSockAddress(const Address &address);
extern Address toAddress(const ::sockaddr_in &address);

#define APPLICATION_INSTANCE_REQUIRED() assert(Application::instance())
}

#endif //NETPP_UTIL_H
