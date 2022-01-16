//
// Created by gaojian on 2022/1/9.
//

#include "http/HttpCode.h"

namespace netpp::http {
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
