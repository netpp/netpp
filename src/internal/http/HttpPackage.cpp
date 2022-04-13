//
// Created by gaojian on 2021/12/19.
//

#include "internal/http/HttpPackage.h"
#include "llhttp.h"
#include "internal/socket/SocketIO.h"
#include <cstring>
#include <ranges>
#include "internal/support/Log.h"
#include "http/HttpRequest.h"
extern "C" {
#include <sys/socket.h>
}

using namespace netpp::http;

#define D_C(parser) \
	DecoderImpl *d = reinterpret_cast<DecoderImpl *>(parser);

namespace netpp::internal::http {
class DecoderImpl {
public:
	DecoderImpl();
	bool parse(std::weak_ptr<ByteArray> &byteArray);
	void resetParse();
	HttpResponse decodedResponse();
	HttpRequest decodedRequest();

public:
	static int onMessageBegin(llhttp_t *parser);
	static int onUrl(llhttp_t*, const char *at, size_t length);
	static int onStatus(llhttp_t*, const char *at, size_t length);
	static int onHeaderField(llhttp_t*, const char *at, size_t length);
	static int onHeaderValue(llhttp_t*, const char *at, size_t length);
	static int onHeadersComplete(llhttp_t *parser);
	/*static int onBody(llhttp_t*, const char *at, size_t length);
	static int onMessageComplete(llhttp_t *parser);
	static int onChunkHeader(llhttp_t *parser);
	static int onChunkComplete(llhttp_t *parser);
	static int onUrlComplete(llhttp_t *parser);
	static int onStatusComplete(llhttp_t *parser);
	static int onHeaderFieldComplete(llhttp_t *parser);
	static int onHeaderValueComplete(llhttp_t *parser);*/

private:
	void setByteArrayLength(internal::socket::ByteArrayReaderWithLock &reader, const char *start, ByteArray::LengthType length) const;
	void getCodeAndProtocolVersion();

private:
	::llhttp_t m_parser;

	std::string headerField;
	RequestMethod method;
	StatusCode statusCode;
	std::string url;
	ProtocolVersion version;
	std::map<std::string, std::string> header;
};

constexpr llhttp_settings_t cb_setting = {
	&DecoderImpl::onMessageBegin,
	&DecoderImpl::onUrl,
	&DecoderImpl::onStatus,
	&DecoderImpl::onHeaderField,
	&DecoderImpl::onHeaderValue,
	&DecoderImpl::onHeadersComplete,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

HttpParser::HttpParser() : m_impl{std::make_unique<DecoderImpl>()} {}

HttpParser::~HttpParser() = default;

std::optional<HttpRequest> HttpParser::decodeRequest(std::weak_ptr<ByteArray> byteArray)
{
	m_impl->resetParse();
	if (m_impl->parse(byteArray))
	{
		HttpRequest request = m_impl->decodedRequest();
		if (request.hasHeader(KnownHeader::ContentLength))
			request.setBody(byteArray.lock());
		else
			request.setBody(std::make_shared<ByteArray>());
		return request;
	}
	else
		return std::nullopt;
}

std::optional<HttpResponse> HttpParser::decodeResponse(std::weak_ptr<ByteArray> byteArray)
{
	m_impl->resetParse();
	if (m_impl->parse(byteArray))
	{
		HttpResponse response = m_impl->decodedResponse();
		if (response.hasHeader(KnownHeader::ContentLength))
			response.setBody(byteArray.lock());
		else
			response.setBody(std::make_shared<ByteArray>());
		return response;
	}
	else
		return std::nullopt;
}

DecoderImpl::DecoderImpl()
		: m_parser{}, method{RequestMethod::UnknownHeader}, statusCode{StatusCode::OK}, version{ProtocolVersion::UnkownProtocol}
{
	::llhttp_init(&m_parser, HTTP_BOTH, &cb_setting);
}

bool DecoderImpl::parse(std::weak_ptr<ByteArray> &byteArray)
{
	internal::socket::ByteArrayReaderWithLock reader(byteArray.lock());
	::iovec *vec = reader.iovec();
	std::size_t nodeSize = reader.iovenLength();

	bool parseSuccess = false;
	if (nodeSize > 0)
	{
		::llhttp_errno placementErr = ::llhttp_execute(&m_parser, static_cast<char *>(vec[0].iov_base), vec[0].iov_len);
		switch (placementErr)
		{
			case HPE_OK:
			case HPE_PAUSED_UPGRADE:	// the header is completed
				getCodeAndProtocolVersion();
				setByteArrayLength(reader, static_cast<char *>(vec[0].iov_base), vec[0].iov_len);
				parseSuccess = true;
				break;
			case HPE_INVALID_METHOD:	// syntax error
			case HPE_INVALID_URL:
			case HPE_INVALID_CONSTANT:
			case HPE_INVALID_VERSION:
			case HPE_INVALID_HEADER_TOKEN:
			case HPE_INVALID_CONTENT_LENGTH:
			case HPE_INVALID_CHUNK_SIZE:
			case HPE_INVALID_STATUS:
			case HPE_INVALID_EOF_STATE:
			case HPE_INVALID_TRANSFER_ENCODING:
				LOG_INFO("Syntax error {}:{}", ::llhttp_errno_name(placementErr), m_parser.reason);
				parseSuccess = false;
				break;
			default:					// header not completed, copy m_nodes and restart
				// TODO: we should implement http parse method, copy m_nodes is not efficient
				for (std::size_t i = 0; i <= nodeSize; i *= 2)
				{
					if (i - nodeSize < i * 2)
						i = nodeSize;
					char *buffer = new char[ByteArray::BufferNodeSize * i];
					ByteArray::LengthType byteArrayLength = 0;
					for (std::size_t j = 0; j < i; ++j)
					{
						std::memcpy(buffer + byteArrayLength, vec[j].iov_base, vec[j].iov_len);
						byteArrayLength += vec[j].iov_len;
					}
					::llhttp_reset(&m_parser);
					::llhttp_errno copyErr = ::llhttp_execute(&m_parser, buffer, byteArrayLength);
					if (copyErr == HPE_PAUSED_UPGRADE)
					{
						getCodeAndProtocolVersion();
						setByteArrayLength(reader, buffer, byteArrayLength);
						parseSuccess = true;
					}
					delete []buffer;
				}
				break;
		}
	}
	return parseSuccess;
}

void DecoderImpl::resetParse()
{
	::llhttp_reset(&m_parser);
	headerField = "";
	method = RequestMethod::UnknownHeader;
	statusCode = StatusCode::OK;
	url = "";
	version = ProtocolVersion::UnkownProtocol;
	header = {};
}

HttpResponse DecoderImpl::decodedResponse()
{
	HttpResponse response;
	response.setStatus(statusCode);
	response.setHttpVersion(version);
	response.setHeader(header);
	return response;
}

HttpRequest DecoderImpl::decodedRequest()
{
	HttpRequest request;
	request.setMethod(method);
	request.setUrl(url);
	request.setHttpVersion(version);
	request.setHeader(header);
	return request;
}

int DecoderImpl::onMessageBegin([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onUrl(llhttp_t *parser, const char *at, size_t length)
{
	D_C(parser)
	d->url = std::string(at, length);
	return 0;
}

int DecoderImpl::onStatus([[maybe_unused]] llhttp_t *parser, [[maybe_unused]] const char *at, [[maybe_unused]] size_t length)
{
	return 0;
}

int DecoderImpl::onHeaderField(llhttp_t *parser, const char *at, size_t length)
{
	D_C(parser)
	d->headerField = std::string(at, length);
	return 0;
}

int DecoderImpl::onHeaderValue(llhttp_t *parser, const char *at, size_t length)
{
	D_C(parser)
	d->header.emplace(d->headerField, std::string(at, length));
	return 0;
}

int DecoderImpl::onHeadersComplete([[maybe_unused]] llhttp_t *parser)
{
	// parse was done after header completed
	return 2;
}

/*int DecoderImpl::onBody([[maybe_unused]] llhttp_t *parser, [[maybe_unused]] const char *at, [[maybe_unused]] size_t length)
{
	return 0;
}

int DecoderImpl::onMessageComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onChunkHeader([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onChunkComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onUrlComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onStatusComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onHeaderFieldComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}

int DecoderImpl::onHeaderValueComplete([[maybe_unused]] llhttp_t *parser)
{
	return 0;
}*/

void DecoderImpl::setByteArrayLength(internal::socket::ByteArrayReaderWithLock &reader, const char *start, ByteArray::LengthType length) const
{
	long parsedLength = m_parser.error_pos - start;
	if (parsedLength > 0 && static_cast<ByteArray::LengthType>(parsedLength) < length)
	{
		reader.adjustByteArray(static_cast<ByteArray::LengthType>(parsedLength));
	}
	else
	{
		LOG_ERROR("Unknown error while parsing http, unexpected pointer");
	}
}

void DecoderImpl::getCodeAndProtocolVersion()
{
	statusCode = static_cast<StatusCode>(m_parser.status_code);
	version = getHttpVersion(m_parser.http_major, m_parser.http_minor);
	switch (static_cast<llhttp_method>(m_parser.method))
	{
		case HTTP_DELETE:	method = RequestMethod::Delete;	break;
		case HTTP_GET:	method = RequestMethod::Get;	break;
		case HTTP_HEAD:	method = RequestMethod::Head;	break;
		case HTTP_POST:	method = RequestMethod::Post;	break;
		case HTTP_PUT:	method = RequestMethod::Put;	break;
		case HTTP_CONNECT:	method = RequestMethod::Connect;	break;
		case HTTP_OPTIONS:	method = RequestMethod::Options;	break;
		case HTTP_TRACE:	method = RequestMethod::Trace;	break;
		case HTTP_COPY:break;
		case HTTP_LOCK:break;
		case HTTP_MKCOL:break;
		case HTTP_MOVE:break;
		case HTTP_PROPFIND:break;
		case HTTP_PROPPATCH:break;
		case HTTP_SEARCH:break;
		case HTTP_UNLOCK:break;
		case HTTP_BIND:break;
		case HTTP_REBIND:break;
		case HTTP_UNBIND:break;
		case HTTP_ACL:break;
		case HTTP_REPORT:break;
		case HTTP_MKACTIVITY:break;
		case HTTP_CHECKOUT:break;
		case HTTP_MERGE:break;
		case HTTP_MSEARCH:break;
		case HTTP_NOTIFY:break;
		case HTTP_SUBSCRIBE:break;
		case HTTP_UNSUBSCRIBE:break;
		case HTTP_PATCH:	method = RequestMethod::Patch;	break;
		case HTTP_PURGE:break;
		case HTTP_MKCALENDAR:break;
		case HTTP_LINK:break;
		case HTTP_UNLINK:break;
		case HTTP_SOURCE:break;
		case HTTP_PRI:break;
		case HTTP_DESCRIBE:break;
		case HTTP_ANNOUNCE:break;
		case HTTP_SETUP:break;
		case HTTP_PLAY:break;
		case HTTP_PAUSE:break;
		case HTTP_TEARDOWN:break;
		case HTTP_GET_PARAMETER:break;
		case HTTP_SET_PARAMETER:break;
		case HTTP_REDIRECT:break;
		case HTTP_RECORD:break;
		case HTTP_FLUSH:break;
	}
}

void HttpPackage::encode(netpp::http::HttpRequest &request)
{
	auto method = getMethodAsString(request.method());
	m_headerField->writeRaw(method.data(), method.length());
	m_headerField->writeRaw(" ", 1);
	// TODO: add url parse support
	std::string url = "/";
	m_headerField->writeString(url);
	m_headerField->writeRaw(" ", 1);
	auto version = getHttpVersionAsString(request.httpVersion());
	m_headerField->writeRaw(version.data(), version.length());
	m_headerField->writeRaw("\r\n", 2);
	for (auto h : request.headers())
	{
		m_headerField->writeRaw(h.first.data(), h.first.length());
		m_headerField->writeRaw(h.second.data(), h.second.length());
		m_headerField->writeRaw("\r\n", 2);
	}
	m_bodyField = request.body();
	ByteArray::LengthType contentLength = m_bodyField->readableBytes();
	if (!request.hasHeader(KnownHeader::ContentLength) && contentLength != 0)
	{
		auto len = getHeaderAsString(KnownHeader::ContentLength);
		m_headerField->writeRaw(len.data(), len.length());
		m_headerField->writeRaw("\r\n", 2);
	}
	m_headerField->writeRaw("\r\n", 2);
}

void HttpPackage::encode(netpp::http::HttpResponse &response)
{
	auto version = getHttpVersionAsString(response.httpVersion());
	m_headerField->writeRaw(version.data(), version.length());
	m_headerField->writeRaw(" ", 1);
	std::string code = std::to_string(static_cast<int>(response.status()));
	m_headerField->writeString(code);
	m_headerField->writeRaw(" ", 1);
	std::string statusString = response.statusString();
	m_headerField->writeString(statusString);
	m_headerField->writeRaw("\r\n", 2);
	for (auto h : response.headers())
	{
		m_headerField->writeRaw(h.first.data(), h.first.length());
		m_headerField->writeRaw(h.second.data(), h.second.length());
		m_headerField->writeRaw("\r\n", 2);
	}
	m_bodyField = response.body();
	ByteArray::LengthType contentLength = m_bodyField->readableBytes();
	if (!response.hasHeader(KnownHeader::ContentLength) && contentLength != 0)
	{
		auto len = getHeaderAsString(KnownHeader::ContentLength);
		m_headerField->writeRaw(len.data(), len.length());
		m_headerField->writeRaw("\r\n", 2);
	}
	m_headerField->writeRaw("\r\n", 2);
}
}
