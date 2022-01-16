//
// Created by gaojian on 2021/12/18.
//

#ifndef NETPP_HTTPREQUEST_H
#define NETPP_HTTPREQUEST_H

#include "HttpCode.h"
#include <string>
#include <map>

namespace netpp::http {
class HttpRequest {
private:
	RequestMethod method;
	std::string url;
	ProtocolVersion version;
	std::map<std::string, std::string> header;
	std::string body;
};
}

#endif //NETPP_HTTPREQUEST_H
