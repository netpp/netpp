//
// Created by gaojian on 2022/1/16.
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
	EXPECT_EQ(request.m_version, http::ProtocolVersion::Http2_0);
	EXPECT_EQ(request.m_method, http::RequestMethod::Get);
}

TEST_F(HttpRequestParseTest, SetRequestTest)
{
}

TEST_F(HttpRequestParseTest, RequestVersionTest)
{
	struct VersionTest {
		std::string test_case_name;
		http::ProtocolVersion version;
		std::string url;
	};
	std::vector<VersionTest> version_suits = {
			{ "v1_0", http::ProtocolVersion::Http1_0, "/" },
			{ "v1_1", http::ProtocolVersion::Http1_1, "/" },
			{ "v2_0", http::ProtocolVersion::Http2_0, "/" },
			{ "v1_4", http::ProtocolVersion::UnkownProtocol, "/" }
	};
	for (auto &v : version_suits)
	{
		std::ifstream fs(std::string("requests/http_request_") + v.test_case_name);
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
			EXPECT_EQ(request->method(), http::RequestMethod::Get);
		}
	}
}

TEST_F(HttpRequestParseTest, RequestWithOutHeaderTest)
{
	struct TestExpect {
		std::string test_case_name;
		http::ProtocolVersion version;
		std::string body;
	};
	std::vector<TestExpect> test_suits = {
			{ "v2_0", http::ProtocolVersion::Http2_0, "" },
			{ "without_header", http::ProtocolVersion::Http2_0, "" },
	};

	for (auto &t : test_suits)
	{
		std::ifstream fs(std::string("requests/http_request_") + t.test_case_name);
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
			EXPECT_EQ(request->httpVersion(), t.version);
			EXPECT_EQ(request->url(), std::string("/"));
			EXPECT_EQ(request->method(), http::RequestMethod::Get);
			auto bodyBuffer = request->body();
			std::string body = bodyBuffer->retrieveString(bodyBuffer->readableBytes());
			EXPECT_EQ(body, t.body);
		}
	}
}

TEST_F(HttpRequestParseTest, RequestHeaderTest)
{
	struct VersionTest {
		std::string test_case_name;
		http::ProtocolVersion version;
		std::string url;
		std::map<std::string, std::string> header;
		std::string body;
	};
	std::vector<VersionTest> version_suits = {
			{
				"header",
				http::ProtocolVersion::Http2_0,
				"/test",
				{
					{ "User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3" },
					{ "Host", "www.example.com" },
					{ "Accept-Language", "en, mi" },
					{ "some_custom_header", "haha" },
					{ "content-length", "13" },
					{ "content-type", "application/json" }
				},
				"{ \"a\" : \"b\" }\r\n"
			},
			{
				"header_without_body",
				http::ProtocolVersion::Http2_0,
				"/test",
				{
					{ "User-Agent", "curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3" },
					{ "Host", "www.example.com" },
					{ "Accept-Language", "en, mi" },
					{ "some_custom_header", "haha" }
				},
				""
			}
	};
	for (auto &v : version_suits)
	{
		std::ifstream fs(std::string("requests/http_request_") + v.test_case_name);
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
