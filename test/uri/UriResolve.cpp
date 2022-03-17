//
// Created by kojiko on 2022/3/16.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriResolve : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

// 5.4.1. Normal Examples https://tools.ietf.org/html/rfc3986#section-5.4.1
TEST_F(UriResolve, ResolveUriNormal)
{
	const Uri baseUri("http://a/b/c/d;p?q");

	EXPECT_EQ(baseUri.resolveUri("g:h"), Uri("g:h"));
	EXPECT_EQ(baseUri.resolveUri("g"), Uri("http://a/b/c/g"));
	EXPECT_EQ(baseUri.resolveUri("./g"), Uri("http://a/b/c/g"));
	EXPECT_EQ(baseUri.resolveUri("g/"), Uri("http://a/b/c/g/"));
	EXPECT_EQ(baseUri.resolveUri("/g"), Uri("http://a/g"));
	EXPECT_EQ(baseUri.resolveUri("//g"), Uri("http://g"));
	EXPECT_EQ(baseUri.resolveUri("?y"), Uri("http://a/b/c/d;p?y"));
	EXPECT_EQ(baseUri.resolveUri("g?y"), Uri("http://a/b/c/g?y"));
	EXPECT_EQ(baseUri.resolveUri("#s"), Uri("http://a/b/c/d;p?q#s"));
	EXPECT_EQ(baseUri.resolveUri("g#s"), Uri("http://a/b/c/g#s"));
	EXPECT_EQ(baseUri.resolveUri("g?y#s"), Uri("http://a/b/c/g?y#s"));
	EXPECT_EQ(baseUri.resolveUri(";x"), Uri("http://a/b/c/;x"));
	EXPECT_EQ(baseUri.resolveUri("g;x"), Uri("http://a/b/c/g;x"));
	EXPECT_EQ(baseUri.resolveUri("g;x?y#s"), Uri("http://a/b/c/g;x?y#s"));
	EXPECT_EQ(baseUri.resolveUri(""), Uri("http://a/b/c/d;p?q"));
	EXPECT_EQ(baseUri.resolveUri("."), Uri("http://a/b/c/"));
	EXPECT_EQ(baseUri.resolveUri("./"), Uri("http://a/b/c/"));
	EXPECT_EQ(baseUri.resolveUri(".."), Uri("http://a/b/"));
	EXPECT_EQ(baseUri.resolveUri("../"), Uri("http://a/b/"));
	EXPECT_EQ(baseUri.resolveUri("../g"), Uri("http://a/b/g"));
	EXPECT_EQ(baseUri.resolveUri("../.."), Uri("http://a/"));
	EXPECT_EQ(baseUri.resolveUri("../../"), Uri("http://a/"));
	EXPECT_EQ(baseUri.resolveUri("../../g"), Uri("http://a/g"));
}

// 5.4.2. Abnormal Examples https://tools.ietf.org/html/rfc3986#section-5.4.2
TEST_F(UriResolve, ResolveUriAbnormal)
{
	const Uri baseUri("http://a/b/c/d;p?q");

	EXPECT_EQ(baseUri.resolveUri("../../../g"), Uri("http://a/g"));
	EXPECT_EQ(baseUri.resolveUri("../../../../g"), Uri("http://a/g"));
	EXPECT_EQ(baseUri.resolveUri("/./g"), Uri("http://a/g"));
	EXPECT_EQ(baseUri.resolveUri("/../g"), Uri("http://a/g"));
	EXPECT_EQ(baseUri.resolveUri("g."), Uri("http://a/b/c/g."));
	EXPECT_EQ(baseUri.resolveUri(".g"), Uri("http://a/b/c/.g"));
	EXPECT_EQ(baseUri.resolveUri("g.."), Uri("http://a/b/c/g.."));
	EXPECT_EQ(baseUri.resolveUri("..g"), Uri("http://a/b/c/..g"));
	EXPECT_EQ(baseUri.resolveUri("./../g"), Uri("http://a/b/g"));
	EXPECT_EQ(baseUri.resolveUri("./g/."), Uri("http://a/b/c/g/"));
	EXPECT_EQ(baseUri.resolveUri("g/./h"), Uri("http://a/b/c/g/h"));
	EXPECT_EQ(baseUri.resolveUri("g/../h"), Uri("http://a/b/c/h"));
	EXPECT_EQ(baseUri.resolveUri("g;x=1/./y"), Uri("http://a/b/c/g;x=1/y"));
	EXPECT_EQ(baseUri.resolveUri("g;x=1/../y"), Uri("http://a/b/c/y"));
	EXPECT_EQ(baseUri.resolveUri("g?y/./x"), Uri("http://a/b/c/g?y/./x"));
	EXPECT_EQ(baseUri.resolveUri("g?y/../x"), Uri("http://a/b/c/g?y/../x"));
	EXPECT_EQ(baseUri.resolveUri("g#s/./x"), Uri("http://a/b/c/g#s/./x"));
	EXPECT_EQ(baseUri.resolveUri("g#s/../x"), Uri("http://a/b/c/g#s/../x"));
	EXPECT_EQ(baseUri.resolveUri("http:g"), Uri("http:g"));
}