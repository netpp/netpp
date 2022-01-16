//
// Created by gaojian on 2021/12/19.
//

#include "http/Parser.h"
#include "llhttp.h"
#include "internal/socket/SocketIO.h"
#include <cstring>
#include <ranges>
#include "internal/support/Log.h"
extern "C" {
#include <sys/socket.h>
}

#define D_C(parser) \
	DecoderImpl *d = reinterpret_cast<DecoderImpl *>(parser);

#define NETPP_UNUSED(x)	(void)(x)

namespace netpp::http {
class DecoderImpl {
public:
	DecoderImpl();
	bool prase(std::weak_ptr<ByteArray> &byteArray);
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
	void setByteArrayLength(internal::socket::ByteArrayIOVecReaderWithLock &reader, const char *start, ByteArray::LengthType length) const;

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

Parser::Parser()
	: m_impl{std::make_unique<DecoderImpl>()}
{
}

Parser::~Parser() = default;

std::optional<HttpRequest> Parser::decodeRequest(std::weak_ptr<ByteArray> byteArray)
{
	if (m_impl->prase(byteArray))
	{
		return m_impl->decodedRequest();
	}
	else
		return std::nullopt;
}

std::optional<HttpResponse> Parser::decodeResponse(std::weak_ptr<ByteArray> byteArray)
{
	if (m_impl->prase(byteArray))
	{
		return m_impl->decodedResponse();
	}
	else
		return std::nullopt;
}

void Parser::encode(std::weak_ptr<ByteArray> byteArray, const HttpResponse &response)
{}

DecoderImpl::DecoderImpl()
		: m_parser{}
{
	::llhttp_init(&m_parser, HTTP_BOTH, &cb_setting);
}

bool DecoderImpl::prase(std::weak_ptr<ByteArray> &byteArray)
{
	internal::socket::ByteArrayIOVecReaderWithLock reader(byteArray.lock());
	::iovec *vec = reader.iovec();
	std::size_t nodeSize = reader.iovenLength();

	if (nodeSize > 0)
	{
		::llhttp_errno placementErr = ::llhttp_execute(&m_parser, static_cast<char *>(vec[0].iov_base),
											  vec[0].iov_len
		);
		switch (placementErr)
		{
			case HPE_PAUSED_UPGRADE:	// only manually pause parse means the header is completed
				setByteArrayLength(reader, static_cast<char *>(vec[0].iov_base), vec[0].iov_len);
				return true;
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
				return false;
			default:					// header not completed, copy nodes and restart
				// TODO: we should implement http parse method, copy nodes is not efficient
				for (std::size_t i = 0; i <= nodeSize; i *= 2)
				{
					if (i - nodeSize < i * 2)
						i = nodeSize;
					char *buffer = new char[ByteArray::BufferNodeSize * i];
					ByteArray::LengthType byteArrayLength = 0;
					for (std::size_t j : std::ranges::iota_view(static_cast<std::size_t>(0), i))
					{
						std::memcpy(buffer + byteArrayLength, vec[j].iov_base, vec[j].iov_len);
						byteArrayLength += vec[j].iov_len;
					}
					::llhttp_reset(&m_parser);
					::llhttp_errno copyErr = ::llhttp_execute(&m_parser, buffer, byteArrayLength);
					delete []buffer;
					if (copyErr == HPE_PAUSED_UPGRADE)
					{
						setByteArrayLength(reader, buffer, byteArrayLength);
						return true;
					}
				}
				return false;
		}
	}
	return false;
}

HttpResponse DecoderImpl::decodedResponse()
{}

HttpRequest DecoderImpl::decodedRequest()
{}

int DecoderImpl::onMessageBegin(llhttp_t *parser)
{
	NETPP_UNUSED(parser);
	return 0;
}

int DecoderImpl::onUrl(llhttp_t *parser, const char *at, size_t length)
{
	D_C(parser)
	d->url = std::string(at, length);
	return 0;
}

int DecoderImpl::onStatus(llhttp_t *parser, const char *at, size_t length)
{
	NETPP_UNUSED(parser);
	NETPP_UNUSED(at);
	NETPP_UNUSED(length);
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

int DecoderImpl::onHeadersComplete(llhttp_t *parser)
{
	D_C(parser)
	d->statusCode = static_cast<StatusCode>(parser->status_code);
	d->version = getHttpVersion(parser->http_major, parser->http_minor);
	switch (static_cast<llhttp_method>(parser->method))
	{
		case HTTP_DELETE:	d->method = RequestMethod::Delete;	break;
		case HTTP_GET:	d->method = RequestMethod::Get;	break;
		case HTTP_HEAD:	d->method = RequestMethod::Head;	break;
		case HTTP_POST:	d->method = RequestMethod::Post;	break;
		case HTTP_PUT:	d->method = RequestMethod::Put;	break;
		case HTTP_CONNECT:	d->method = RequestMethod::Connect;	break;
		case HTTP_OPTIONS:	d->method = RequestMethod::Options;	break;
		case HTTP_TRACE:	d->method = RequestMethod::Trace;	break;
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
		case HTTP_PATCH:	d->method = RequestMethod::Patch;	break;
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

void DecoderImpl::setByteArrayLength(internal::socket::ByteArrayIOVecReaderWithLock &reader, const char *start, ByteArray::LengthType length) const
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
}
