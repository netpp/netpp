//
// Created by gaojian on 2021/12/19.
//

#ifndef NETPP_HTTPCODE_H
#define NETPP_HTTPCODE_H

#include <string_view>

#define NETPP_HTTP_PROTOCOL_VERSION(XX) \
    XX(UnkownProtocol, 0, 0)	\
	XX(Http1_0, 1, 0)	\
	XX(Http1_1, 1, 1)	\
	XX(Http2_0, 2, 0)

#define NETPP_HTTP_METHOD(XX) \
    XX(UnknownHeader, "UNKNOWN_HEADER")	\
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
	XX(Continue, 100, "Continue")			\
	XX(SwitchingProtocols, 101, "Switching Protocols")	\
	XX(OK, 200, "OK")					\
	XX(Created, 201, "Created")			\
	XX(Accepted, 202, "Accepted")			\
	XX(NonAuthoritativeInformation, 203, "Non-Authoritative Information")	\
	XX(NoContent, 204, "No Content")			\
	XX(ResetContent, 205, "Reset Content")		\
	XX(PartialContent, 206, "Partial Content")		\
	XX(MultipleChoices, 300, "Multiple Choices")	\
	XX(MovedPermanently, 301, "Moved Permanently")	\
	XX(Found, 302, "Found")           \
	XX(SeeOther, 303, "See Other")	\
	XX(NotModified, 304, "Not Modified")	\
	XX(UseProxy, 305, "Use Proxy")			\
	XX(Unused, 306, "Unused")				\
	XX(TemporaryRedirect, 307, "Temporary Redirect")	\
	XX(BadRequest, 400, "Bad Request")			\
	XX(Unauthorized, 401, "Unauthorized")		\
	XX(PaymentRequired, 402, "Payment Required")	\
	XX(Forbidden, 403, "Forbidden")			\
	XX(NotFound, 404, "Not Found")			\
	XX(MethodNotAllowed, 405, "Method Not Allowed")	\
	XX(NotAcceptable, 406, "Not Acceptable")		\
	XX(ProxyAuthenticationRequired, 407, "Proxy Authentication Required")	\
	XX(RequestTimeout, 408, "Request Time-out")		\
	XX(Conflict, 409, "Conflict")			\
	XX(Gone, 410, "Gone")				\
	XX(LengthRequired, 411, "Length Required")		\
	XX(PreconditionFailed, 412, "Precondition Failed")	\
	XX(RequestEntityTooLarge, 413, "Request Entity Too Large")	\
	XX(RequestURITooLarge, 414, "Request-URI Too Large")	\
	XX(UnsupportedMediaType, 415, "Unsupported Media Type")	\
	XX(RequestedRangeNotSatisfiable, 416, "Requested range not satisfiable")	\
	XX(ExpectationFailed, 417, "Expectation Failed")	\
	XX(InternalServerError, 500, "Internal Server Error")\
	XX(NotImplemented, 501, "Not Implemented")		\
	XX(BadGateway, 502, "Bad Gateway")			\
	XX(ServiceUnavailable, 503, "Service Unavailable")	\
	XX(GatewayTimeout, 504, "Gateway Time-out")		\
	XX(HTTPVersionNotSupported, 505, "HTTP Version not supported")


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
const std::string_view &getHeaderAsString(KnownHeader header);
ProtocolVersion getHttpVersion(int major, int minor);
const std::string_view &getStatusAsString(StatusCode code);
}

#endif //NETPP_HTTPCODE_H
