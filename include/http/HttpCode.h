//
// Created by gaojian on 2021/12/19.
//

#ifndef NETPP_HTTPCODE_H
#define NETPP_HTTPCODE_H

#include <string_view>

#define NETPP_HTTP_PROTOCOL_VERSION(XX) \
	XX(Http1_0, 1, 0)	\
	XX(Http1_1, 1, 1)	\
	XX(Http2_0, 2, 0)

#define NETPP_HTTP_METHOD(XX)	\
	XX(Get, "GET")			\
	XX(Head, "HEAD")		\
	XX(Post, "POST")		\
	XX(Put, "PUT")			\
	XX(Delete, "DELETE")	\
	XX(Connect, "CONNECT")	\
	XX(Options, "OPTIONS")	\
	XX(Trace, "TRACE")		\
	XX(Patch, "PATCH")

#define NETPP_HTTP_HEADER(XX)					\
	XX(CacheControl, "cache-control")			\
	XX(Expect, "expect")						\
	XX(Host, "host")							\
	XX(MaxForwards, "max-forwards")				\
	XX(Pragma, "pragma")						\
	XX(Range, "range")							\
	XX(TE, "te")								\
	XX(IfMatch, "if-match")						\
	XX(IfNoneMatch, "if-none-match")			\
	XX(IfModifiedSince, "if-modified-since")	\
	XX(IfUnmodifiedSince, "if-unmodified-since")\
	XX(IfRange, "if-range")						\
	XX(Accept, "accept")						\
	XX(AcceptCharset, "accept-charset")			\
	XX(AcceptEncoding, "accept-encoding")		\
	XX(AcceptLanguage, "accept-language")		\
	XX(Authorization, "authorization")			\
	XX(ProxyAuthorization, "proxy-authorization")	\
	XX(From, "from")							\
	XX(Referer, "referer")						\
	XX(UserAgent, "user-agent")					\
	XX(Age, "age")								\
	XX(Expires, "expires")						\
	XX(Date, "date")							\
	XX(Location, "location")					\
	XX(RetryAfter, "retry-after")				\
	XX(Vary, "vary")							\
	XX(Warning, "warning")						\
	XX(ETag, "etag")							\
	XX(LastModified, "last-modified")			\
	XX(WWWAuthenticate, "WWW-authenticate")		\
	XX(ProxyAuthenticate, "proxy-authenticate")	\
	XX(AcceptRanges, "accept-ranges")			\
	XX(Allow, "allow")							\
	XX(Server, "server")						\
	XX(ContentEncoding, "content-encoding")		\
	XX(ContentLength, "content-length")			\
	XX(ContentType, "content-type")

#define NETPP_HTTP_STATUS_CODE(XX)	\
	XX(Continue, 100, "")			\
	XX(SwitchingProtocols, 101, "")	\
	XX(OK, 200, "")					\
	XX(Created, 201, "")			\
	XX(Accepted, 202, "")			\
	XX(NonAuthoritativeInformation, 203, "")	\
	XX(NoContent, 204, "")			\
	XX(ResetContent, 205, "")		\
	XX(PartialContent, 206, "")		\
	XX(MultipleChoices, 300, "")	\
	XX(MovedPermanently, 301, "")	\
	XX(Found, 302, "")				\
	XX(UseProxy, 305, "")			\
	XX(Unused, 306, "")				\
	XX(TemporaryRedirect, 307, "")	\
	XX(BadRequest, 400, "")			\
	XX(Unauthorized, 401, "")		\
	XX(PaymentRequired, 402, "")	\
	XX(Forbidden, 403, "")			\
	XX(NotFound, 404, "")			\
	XX(MethodNotAllowed, 405, "")	\
	XX(NotAcceptable, 406, "")		\
	XX(ProxyAuthenticationRequired, 407, "")	\
	XX(RequestTimeout, 408, "")		\
	XX(Conflict, 409, "")			\
	XX(Gone, 410, "")				\
	XX(LengthRequired, 411, "")		\
	XX(PreconditionFailed, 412, "")	\
	XX(RequestEntityTooLarge, 413, "")	\
	XX(RequestURITooLarge, 414, "")	\
	XX(UnsupportedMediaType, 415, "")	\
	XX(RequestedRangeNotSatisfiable, 416, "")	\
	XX(ExpectationFailed, 417, "")	\
	XX(InternalServerError, 500, "")\
	XX(NotImplemented, 501, "")		\
	XX(BadGateway, 502, "")			\
	XX(ServiceUnavailable, 503, "")	\
	XX(GatewayTimeout, 504, "")		\
	XX(HTTPVersionNotSupported, 505, "")


namespace netpp::http {
#define NETPP_PROTOCOL_DEFINE(version, major_version, minor_version) version,
enum class ProtocolVersion {
	NETPP_HTTP_PROTOCOL_VERSION(NETPP_PROTOCOL_DEFINE)
};

#define NETPP_HTTP_METHOD_DEFINE(method, as_string) method,
enum class RequestMethod {
	NETPP_HTTP_METHOD(NETPP_HTTP_METHOD_DEFINE)
};

#define NETPP_HTTP_HEADER_DEFINE(header, as_string) header,
enum class KnownHeader {
	NETPP_HTTP_HEADER(NETPP_HTTP_HEADER_DEFINE)
};

#define NETPP_HTTP_STATUS_CODE_DEFINE(status, code, err_string) status = (code),
enum class StatusCode {
	NETPP_HTTP_STATUS_CODE(NETPP_HTTP_STATUS_CODE_DEFINE)
};
const std::string_view &getHeader(KnownHeader header);
ProtocolVersion getHttpVersion(int major, int minor);
}

#endif //NETPP_HTTPCODE_H
