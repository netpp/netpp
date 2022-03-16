//
// Created by kojiko on 2022/3/16.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriConversions : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriConversions, ToStringConversion)
{
	utf8string encoded = UriCodec::encode("http://testname.com/%%?qstring");
	Uri u1("http://testname.com/%25%25?qstring");

	EXPECT_EQ(UriCodec::decode(encoded), UriCodec::decode(u1.toString()));
}

TEST_F(UriConversions, ToEncodedString)
{
	utf8string encoded = UriCodec::encode("http://testname.com/%%?qstring");
	Uri u("http://testname.com/%25%25?qstring");

	EXPECT_EQ(encoded, u.toString());
}

TEST_F(UriConversions, EmptyToString)
{
	Uri u;
	EXPECT_EQ("/", u.toString());
}