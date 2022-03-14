//
// Created by gaojian on 2022/1/16.
//
#include <gtest/gtest.h>
#include <fstream>
#define private public
#define protected public
#include "ByteArray.h"
#include "internal/http/HttpPackage.h"
#include "http/HttpRequest.h"
#undef private
#undef protected

using namespace netpp;

class HttpRequestParseTest : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(HttpRequestParseTest, DefaultRequestTest)
{
	http::HttpRequest request;
	EXPECT_TRUE(request.url().empty());
	EXPECT_EQ(request.method(), http::RequestMethod::Get);
	std::string url("/test");
	request.setUrl(url);
	request.setMethod(http::RequestMethod::Post);
	EXPECT_EQ(request.url(), url);
	EXPECT_EQ(request.method(), http::RequestMethod::Post);
}

TEST_F(HttpRequestParseTest, RequestParseTest)
{
	struct RequestContentTest {
		std::string test_case_name;
		http::ProtocolVersion version;
		std::string url;
		std::map<std::string, std::string> header;
		std::string body;
	};
	std::vector<RequestContentTest> version_suits = {
			{ "header", http::ProtocolVersion::Http2_0, "/test",
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
			{ "header_without_body", http::ProtocolVersion::Http2_0, "/test",
				{
					{ "User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3" },
					{ "Host", "www.example.com" },
					{ "Accept-Language", "en, mi" },
					{ "some_custom_header", "haha" }
				},
				""
			},
			{ "v1_0", http::ProtocolVersion::Http1_0, "/", {}, "" },
			{ "v1_1", http::ProtocolVersion::Http1_1, "/", {}, "" },
			{ "v2_0", http::ProtocolVersion::Http2_0, "/", {}, "" },
			{ "v1_4", http::ProtocolVersion::UnkownProtocol, "/", {}, "" },
			{ "without_header", http::ProtocolVersion::Http2_0, "/", {}, "" }
	};
	for (auto &v : version_suits)
	{
		std::ifstream fs("requests/" + v.test_case_name + ".http_request");
		EXPECT_TRUE(fs.is_open());
		if (fs.is_open())
		{
			std::shared_ptr<ByteArray> buffer = std::make_shared<ByteArray>();
			char c;
			while ((c = static_cast<char>(fs.get())) != EOF)
				buffer->writeRaw(reinterpret_cast<char *>(&c), sizeof(decltype(c)));
			netpp::internal::http::HttpParser parser;
			auto request = parser.decodeRequest(buffer);
			EXPECT_TRUE(request.has_value());
			EXPECT_EQ(request->httpVersion(), v.version);
			EXPECT_EQ(request->url(), v.url);
			for (auto &h : v.header)
			{
				EXPECT_TRUE(request->hasHeader(h.first));
				EXPECT_EQ(request->getHeader(h.first), h.second);
			}
			EXPECT_EQ(request->method(), http::RequestMethod::Get);
			auto bodyBuffer = request->body();
			std::string body = bodyBuffer->retrieveString(bodyBuffer->readableBytes());
			EXPECT_EQ(body, v.body);
		}
	}
}
