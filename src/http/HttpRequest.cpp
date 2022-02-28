//
// Created by gaojian on 2021/12/18.
//

#include "http/HttpRequest.h"
#include "ByteArray.h"

namespace netpp::http {
HttpRequest::HttpRequest()
		: m_method{RequestMethod::Get}, m_version{ProtocolVersion::Http2_0}
{}

void HttpRequest::setHeader(std::map<std::string, std::string> header)
{
	m_header = std::move(header);
}

void HttpRequest::setHeader(std::map<std::string, std::string> &&header)
{
	m_header = std::move(header);
}

void HttpRequest::addHeader(KnownHeader header, const std::string &value)
{
	const std::string_view &headerString = getHeaderAsString(header);
	addRawHeader(std::string(headerString), std::string(value));
}

void HttpRequest::addHeader(KnownHeader header, std::string &&value)
{
	const std::string_view &headerString = getHeaderAsString(header);
	addRawHeader(std::string(headerString), std::move(value));
}

void HttpRequest::addRawHeader(const std::string &header, const std::string &value)
{
	addRawHeader(std::string(header), std::string(value));
}

void HttpRequest::addRawHeader(const std::string &header, std::string &&value)
{
	addRawHeader(std::string(header), std::move(value));
}

void HttpRequest::addRawHeader(std::string &&header, const std::string &value)
{
	addRawHeader(std::move(header), std::string(value));
}

void HttpRequest::addRawHeader(std::string &&header, std::string &&value)
{
	// add header implement here
	m_header.emplace(std::move(header), std::move(value));
}

bool HttpRequest::hasHeader(KnownHeader header)
{
	return hasHeader(std::string(getHeaderAsString(header)));
}

bool HttpRequest::hasHeader(const std::string &header)
{
	return hasHeader(std::string(header));
}

bool HttpRequest::hasHeader(std::string &&header)
{
	auto it = m_header.find(header);
	return it != m_header.end();
}

void HttpRequest::setBody(std::shared_ptr<ByteArray> body)
{
	m_bodyBuffer = std::move(body);
}

std::shared_ptr<ByteArray> HttpRequest::body() const
{
	return m_bodyBuffer;
}
}
