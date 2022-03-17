//
// Created by kojiko on 2022/3/15.
//

#ifndef NETPP_TYPES_H
#define NETPP_TYPES_H

#include <string>
#include <sstream>

namespace netpp::http {
using utf8string = std::string;

inline const utf8string& toUtf8String(const utf8string& val) { return val; }

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
}

#endif //NETPP_TYPES_H
