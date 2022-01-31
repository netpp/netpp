//
// Created by gaojian on 2022/1/9.
//

#include "http/HttpCode.h"

namespace netpp::http {
const std::string_view &getHeader(KnownHeader header)
{
	// avoid construct
#define NETPP_HTTP_HEADER_TO_STRING_STATIC_VAR(header, as_string) \
	static constexpr std::string_view h##header{as_string};

	NETPP_HTTP_HEADER(NETPP_HTTP_HEADER_TO_STRING_STATIC_VAR);
	static constexpr std::string_view hUnknown;

#define NETPP_HTTP_HEADER_TO_STRING(header, as_string) \
	case KnownHeader::header: return h##header;

	switch (header)
	{
		NETPP_HTTP_HEADER(NETPP_HTTP_HEADER_TO_STRING)
	}
	return hUnknown;
}

ProtocolVersion getHttpVersion(int major, int minor)
{
	switch (major * 10 + minor)
	{
		case 10:	return ProtocolVersion::Http1_0;
		case 11:	return ProtocolVersion::Http1_1;
		case 20:	return ProtocolVersion::Http2_0;
		default:	return ProtocolVersion::Http1_0;
	}
}
}
