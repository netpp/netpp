//
// Created by gaojian on 2022/1/16.
//
#ifdef NETPP_HTTP_SUPPORT
#include <gtest/gtest.h>
#define private public
#define protected public
#include "ByteArray.h"
#include "internal/http/HttpParser.h"
#include "http/HttpRequest.h"
#undef private
#undef protected

struct HttpSet {
	const char *httpContext;
	const bool expectSuccess;
	const int httpVersion;
//	const
};
static constexpr const char *request = "";

class HttpTest : public testing::Test {
public:
	static int onConnectedCount;
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}
};

TEST_F(HttpTest, ParseRequestTest)
{
	netpp::internal::http::HttpParser parser;
	std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
//	byteArray.writeRaw("");

	std::optional<netpp::http::HttpRequest> request = parser.decodeRequest(byteArray);
	EXPECT_FALSE(request.has_value());
}
#endif