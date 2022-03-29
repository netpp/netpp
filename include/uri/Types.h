//
// Created by gaojian on 2022/3/15.
//

#ifndef NETPP_TYPES_H
#define NETPP_TYPES_H

#include <string>
#include <sstream>

namespace netpp::uri {
/**
 * @brief The string support utf8 encoding
 */
using utf8string = std::string;

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
}

#endif //NETPP_TYPES_H
