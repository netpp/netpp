//
// Created by gaojian on 2022/3/8.
//

#include <gtest/gtest.h>
#include <fstream>
#define private public
#define protected public
#include "ByteArray.h"
#include "internal/http/HttpParser.h"
#include "http/HttpRequest.h"
#undef private
#undef protected

using namespace netpp;

class HttpResponseParseTest : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(HttpResponseParseTest, DefaultResponseTest)
{
	http::HttpResponse response;
	EXPECT_EQ(response.status(), http::StatusCode::OK);
	EXPECT_EQ(response.statusString(), "OK");

	response.setStatus(http::StatusCode::Continue);
	EXPECT_EQ(response.status(), http::StatusCode::Continue);
	EXPECT_EQ(response.statusString(), "Continue");
}

TEST_F(HttpResponseParseTest, ResponseParseTest)
{
	struct ResponseContentTest {
		std::string test_case_name;
		http::ProtocolVersion version;
		http::StatusCode statusCode;
		std::string status;
		std::map<std::string, std::string> header;
		std::string body;
	};
	std::vector<ResponseContentTest> version_suits = {
			{ "header", http::ProtocolVersion::Http2_0, http::StatusCode::OK, "OK",
			  {
				{ "User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3" },
				{ "Host", "www.example.com" },
				{ "Accept-Language", "en, mi" },
				{ "some_custom_header", "haha" },
				{ "Content-Length", "13" },
				{ "Content-Type", "application/json" }
				},
				"{ \"a\" : \"b\" }\r\n"
			},
			{ "header_without_body", http::ProtocolVersion::Http2_0, http::StatusCode::OK, "OK",
			  {
				{ "User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3" },
				{ "Host", "www.example.com" },
				{ "Accept-Language", "en, mi" },
				{ "some_custom_header", "haha" }
				},
				""
			},
			{ "v1_0", http::ProtocolVersion::Http1_0, http::StatusCode::OK, "OK", 
				{
					{ "Date", "Mon, 27 Jul 2009 12:28:53 GMT" },
					{ "Server", "Apache/2.2.14 (Win32)" },
					{ "Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT" },
					{ "Content-Length", "88" },
					{ "Content-Type", "text/html" },
					{ "Connection", "Closed" }
				}, 
				"<html>\r\n<body>\r\n<h1>Hello world!</h1>\r\n</body>\r\n</html>\r\n"
			},
			{ "v1_1", http::ProtocolVersion::Http1_1, http::StatusCode::OK, "OK", 
				{
					{ "Date", "Mon, 27 Jul 2009 12:28:53 GMT" },
					{ "Server", "Apache/2.2.14 (Win32)" },
					{ "Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT" },
					{ "Content-Length", "88" },
					{ "Content-Type", "text/html" },
					{ "Connection", "Closed" }
				}, 
				"<html>\r\n<body>\r\n<h1>Hello world!</h1>\r\n</body>\r\n</html>\r\n"
			},
			{ "v2_0", http::ProtocolVersion::Http2_0, http::StatusCode::OK, "OK", 
				{
					{ "Date", "Mon, 27 Jul 2009 12:28:53 GMT" },
					{ "Server", "Apache/2.2.14 (Win32)" },
					{ "Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT" },
					{ "Content-Length", "88" },
					{ "Content-Type", "text/html" },
					{ "Connection", "Closed" }
				}, 
				"<html>\r\n<body>\r\n<h1>Hello world!</h1>\r\n</body>\r\n</html>\r\n"
			},
			{ "v1_4", http::ProtocolVersion::UnkownProtocol, http::StatusCode::OK, "OK", 
				{
					{ "Date", "Mon, 27 Jul 2009 12:28:53 GMT" },
					{ "Server", "Apache/2.2.14 (Win32)" },
					{ "Last-Modified", "Wed, 22 Jul 2009 19:15:56 GMT" },
					{ "Content-Length", "88" },
					{ "Content-Type", "text/html" },
					{ "Connection", "Closed" }
				}, 
				"<html>\r\n<body>\r\n<h1>Hello world!</h1>\r\n</body>\r\n</html>\r\n"
			},
			{ "without_header", http::ProtocolVersion::Http2_0, http::StatusCode::OK, "OK", {}, "" }
	};
	for (auto &v : version_suits)
	{
		std::ifstream fs("response/" + v.test_case_name + ".http_response");
		EXPECT_TRUE(fs.is_open());
		if (fs.is_open())
		{
			std::shared_ptr<ByteArray> buffer = std::make_shared<ByteArray>();
			char c;
			while ((c = static_cast<char>(fs.get())) != EOF)
				buffer->writeRaw(reinterpret_cast<char *>(&c), sizeof(decltype(c)));
			netpp::internal::http::HttpParser parser;
			auto response = parser.decodeResponse(buffer);
			EXPECT_TRUE(response.has_value());
			EXPECT_EQ(response->httpVersion(), v.version);
			EXPECT_EQ(response->status(), v.statusCode);
			EXPECT_EQ(response->statusString(), v.status);
			for (auto &h : v.header)
			{
				EXPECT_TRUE(response->hasHeader(h.first));
				EXPECT_EQ(response->getHeader(h.first), h.second);
			}
			auto bodyBuffer = response->body();
			std::string body = bodyBuffer->retrieveString(bodyBuffer->readableBytes());
			EXPECT_EQ(body, v.body);
		}
	}
}
