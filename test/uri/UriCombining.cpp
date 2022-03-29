//
// Created by gaojian on 2022/3/16.
//

#include <gtest/gtest.h>
#include "uri/Uri.h"

using namespace netpp::uri;

class UriCombining : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriCombining, AppendPath)
{
	utf8string uri_str = "http://testname.com/path?baz";
	Uri combined(uri_str);
	combined.appendPath("/baz");

	EXPECT_EQ(Uri("http://testname.com/path/baz?baz"), combined);
}

TEST_F(UriCombining, AppendEmptyPath)
{
	utf8string uri_str("http://fakeuri.net");
	Uri u(uri_str);
	Uri combined(u);
	combined.appendPath("");

	EXPECT_EQ(u, combined);
}

TEST_F(UriCombining, AppendQuery)
{
	utf8string uri_str("http://testname.com/path1?key1=value2");
	Uri combined(uri_str);
	combined.appendQuery(Uri("http://testname2.com/path2?key2=value3").query());

	EXPECT_EQ("http://testname.com/path1?key1=value2&key2=value3", combined.toString());
}

TEST_F(UriCombining, AppendEmptyQuery)
{
	utf8string uri_str("http://fakeuri.org/?key=value");
	Uri u(uri_str);
	Uri combined(u);
	combined.appendQuery("");

	EXPECT_EQ(u, combined);
}
