//
// Created by gaojian on 2021/12/18.
//

#include "http/HttpRequest.h"
#include "ByteArray.h"

namespace netpp::http {
HttpBaseRequest::HttpBaseRequest()
: m_version{ProtocolVersion::Http2_0}
{}

void HttpBaseRequest::setHeader(std::map<std::string, std::string> header)
{
	m_header = std::move(header);
}

void HttpBaseRequest::setHeader(std::map<std::string, std::string> &&header)
{
	m_header = std::move(header);
}

void HttpBaseRequest::addHeader(KnownHeader header, const std::string &value)
{
	const std::string_view &headerString = getHeaderAsString(header);
	addRawHeader(std::string(headerString), std::string(value));
}

void HttpBaseRequest::addHeader(KnownHeader header, std::string &&value)
{
	const std::string_view &headerString = getHeaderAsString(header);
	addRawHeader(std::string(headerString), std::move(value));
}

void HttpBaseRequest::addRawHeader(const std::string &header, const std::string &value)
{
	addRawHeader(std::string(header), std::string(value));
}

void HttpBaseRequest::addRawHeader(const std::string &header, std::string &&value)
{
	addRawHeader(std::string(header), std::move(value));
}

void HttpBaseRequest::addRawHeader(std::string &&header, const std::string &value)
{
	addRawHeader(std::move(header), std::string(value));
}

void HttpBaseRequest::addRawHeader(std::string &&header, std::string &&value)
{
	// add header implement here
	m_header.emplace(std::move(header), std::move(value));
}

std::string HttpBaseRequest::getHeader(KnownHeader header) const
{
	return getHeader(std::string(getHeaderAsString(header)));
}

std::string HttpBaseRequest::getHeader(const std::string &header) const
{
	return getHeader(std::string(header));
}

std::string HttpBaseRequest::getHeader(std::string &&header) const
{
	auto it = m_header.find(header);
	if (it != m_header.end())
		return it->second;
	return "";
}

bool HttpBaseRequest::hasHeader(KnownHeader header) const
{
	return hasHeader(std::string(getHeaderAsString(header)));
}

bool HttpBaseRequest::hasHeader(const std::string &header) const
{
	return hasHeader(std::string(header));
}

bool HttpBaseRequest::hasHeader(std::string &&header) const
{
	auto it = m_header.find(header);
	return it != m_header.end();
}

void HttpBaseRequest::setBody(std::shared_ptr<ByteArray> body)
{
	m_bodyBuffer = std::move(body);
}

std::shared_ptr<ByteArray> HttpBaseRequest::body()
{
	return m_bodyBuffer;
}

HttpRequest::HttpRequest()
	: m_method{RequestMethod::Get}
{}

HttpResponse::HttpResponse()
	: m_statusCode{StatusCode::OK}, m_status{getStatusAsString(StatusCode::OK)}
{}

void HttpResponse::setStatus(StatusCode code)
{
	m_statusCode = code;
	m_status = getStatusAsString(code);
}
}
