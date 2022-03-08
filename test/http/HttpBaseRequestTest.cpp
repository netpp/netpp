//
// Created by gaojian on 2022/3/8.
//
#include <gtest/gtest.h>
#include <fstream>
#define private public
#define protected public
#include "ByteArray.h"
#include "http/HttpRequest.h"
#undef private
#undef protected

using namespace netpp;

class HttpBaseRequestTest : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(HttpBaseRequestTest, DefaultRequestTest)
{
	http::HttpBaseRequest baseRequest;
	EXPECT_EQ(baseRequest.httpVersion(), http::ProtocolVersion::Http2_0);
	baseRequest.setHttpVersion(http::ProtocolVersion::Http1_1);
	EXPECT_EQ(baseRequest.httpVersion(), http::ProtocolVersion::Http1_1);

	std::string accept = "application/json";
	baseRequest.addHeader(http::KnownHeader::Accept, accept);
	EXPECT_TRUE(baseRequest.hasHeader(http::KnownHeader::Accept));
	EXPECT_EQ(baseRequest.getHeader(http::KnownHeader::Accept), accept);
}
