//
// Created by gaojian on 2021/12/19.
//

#ifndef NETPP_PARSER_H
#define NETPP_PARSER_H

#include <optional>
#include <memory>
#include "HttpRequest.h"
#include "ByteArray.h"
#include "HttpResponse.h"

namespace netpp::http {
class DecoderImpl;
class Parser {
public:
	Parser();
	~Parser();
	std::optional<HttpRequest> decodeRequest(std::weak_ptr<ByteArray> byteArray);
	std::optional<HttpResponse> decodeResponse(std::weak_ptr<ByteArray> byteArray);
	void encode(std::weak_ptr<ByteArray> byteArray, const HttpRequest &request);
	void encode(std::weak_ptr<ByteArray> byteArray, const HttpResponse &response);

private:
	std::unique_ptr<DecoderImpl> m_impl;
};
}

#endif //NETPP_PARSER_H
