//
// Created by kojiko on 2022/3/16.
//
#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriAccessor : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriAccessor, AuthorityString)
{
	Uri u("http://testname.com:81/path?baz");
	Uri a = u.authority();

	EXPECT_EQ("/path", u.path());
	EXPECT_EQ("http", a.scheme());
	EXPECT_EQ("testname.com", a.host());
	EXPECT_EQ(81, a.port());
	Uri tmp("http://testname.com:81");
	EXPECT_EQ(tmp, a);
}
