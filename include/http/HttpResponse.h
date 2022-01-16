//
// Created by gaojian on 2021/12/18.
//

#ifndef NETPP_HTTPRESPONSE_H
#define NETPP_HTTPRESPONSE_H

#include "HttpCode.h"
#include <string>
#include <map>

namespace netpp::http {
class HttpResponse {
private:
	ProtocolVersion version;
	StatusCode statusCode;
	std::string status;
	std::map<std::string, std::string> customHeader;
	std::string body;
};
}

#endif //NETPP_HTTPRESPONSE_H
