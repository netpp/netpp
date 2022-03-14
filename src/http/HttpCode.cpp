//
// Created by gaojian on 2022/1/9.
//

#include "http/HttpCode.h"

namespace netpp::http {
const std::string_view &getHeaderAsString(KnownHeader header)
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

#undef NETPP_HTTP_HEADER_TO_STRING_STATIC_VAR
#undef NETPP_HTTP_HEADER_TO_STRING
}

const std::string_view &getStatusAsString(StatusCode code)
{
	// avoid construct
#define NETPP_HTTP_STATUS_TO_STRING_STATIC_VAR(status, code, err_string) \
	static constexpr std::string_view s##status{err_string};

	NETPP_HTTP_STATUS_CODE(NETPP_HTTP_STATUS_TO_STRING_STATIC_VAR)

#define NETPP_HTTP_STATUS_TO_STRING(status, code, err_string) \
	case StatusCode::status: return s##status;

	switch (code)
	{
		NETPP_HTTP_STATUS_CODE(NETPP_HTTP_STATUS_TO_STRING)
	}
	return sOK;
	
#undef NETPP_HTTP_STATUS_TO_STRING_STATIC_VAR
#undef NETPP_HTTP_STATUS_TO_STRING
}

const std::string_view &getMethodAsString(RequestMethod method)
{
#define NETPP_HTTP_METHOD_TO_STRING_STATIC_VAR(method, as_string) \
	static constexpr std::string_view m##method{as_string};

	NETPP_HTTP_METHOD(NETPP_HTTP_METHOD_TO_STRING_STATIC_VAR)

#define NETPP_HTTP_METHOD_TO_STRING(method, as_string) \
	case RequestMethod::method: return m##method;

	switch (method)
	{
		NETPP_HTTP_METHOD(NETPP_HTTP_METHOD_TO_STRING)
	}
	return mGet;
	
#undef NETPP_HTTP_METHOD_TO_STRING_STATIC_VAR
#undef NETPP_HTTP_METHOD_TO_STRING
}

const std::string_view &getHttpVersionAsString(ProtocolVersion version)
{
#define NETPP_HTTP_PROTOCOL_TO_STRING_STATIC_VAR(version, major_version, minor_version) \
	static constexpr std::string_view v##version{"HTTP"#major_version"."#minor_version};

	NETPP_HTTP_PROTOCOL_VERSION(NETPP_HTTP_PROTOCOL_TO_STRING_STATIC_VAR)
	
#define NETPP_HTTP_PROTOCOL_TO_STRING(version, major_version, minor_version) \
	case ProtocolVersion::version: return v##version;

	switch (version)
	{
		NETPP_HTTP_PROTOCOL_VERSION(NETPP_HTTP_PROTOCOL_TO_STRING)
	}
	return vHttp2_0;
}

ProtocolVersion getHttpVersion(int major, int minor)
{
	switch (major * 10 + minor)
	{
		case 10:	return ProtocolVersion::Http1_0;
		case 11:	return ProtocolVersion::Http1_1;
		case 20:	return ProtocolVersion::Http2_0;
		default:	return ProtocolVersion::UnkownProtocol;
	}
}
}
