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
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(HttpRequestParseTest, DefaultRequest)
{
	http::HttpRequest request;
	EXPECT_EQ(request.m_version, http::ProtocolVersion::Http2_0);
	EXPECT_EQ(request.m_method, http::RequestMethod::Get);
}

TEST_F(HttpRequestParseTest, RequestVersionTest)
{
	struct VersionTest {
		std::string test_case_name;
		http::ProtocolVersion version;
	};
	std::vector<VersionTest> version_suits = {
			{ "v1_0", http::ProtocolVersion::Http1_0 },
			{ "v1_1", http::ProtocolVersion::Http1_1 },
			{ "v2_0", http::ProtocolVersion::Http2_0 },
			{ "v1_4", http::ProtocolVersion::UnkownProtocol }
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
//			http::HttpRequest request;
			auto request = parser.decodeRequest(buffer);
			EXPECT_TRUE(request.has_value());
			EXPECT_EQ(request->httpVersion(), v.version);
			EXPECT_EQ(request->url(), std::string("/"));
			EXPECT_EQ(request->method(), http::RequestMethod::Get);
		}
	}
}
