//
// Created by kojiko on 2022/3/16.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriOperator : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriOperator, UriBasicEquality)
{
	EXPECT_EQ(Uri(""), Uri(""));

	Uri u1("http://localhost:80/path1");
	Uri u2("http://localhost:80/path1");
	EXPECT_EQ(u1, u2);
}

TEST_F(UriOperator, UriDecodedEquality)
{
	Uri u3("http://localhost:80");
	u3.setPath("path 1", true);

	Uri u4("http://localhost:80/path%201");
	EXPECT_EQ(u3, u4);

	Uri u5("http://localhost:80/pat%68a1");
	Uri u6("http://localhost:80/patha1");
	EXPECT_EQ(u5, u6);

	Uri u9("http://localhost:80/patha1?name=first#t%65st");
	Uri u10("http://localhost:80/patha1?name=first#test");
	EXPECT_EQ(u9, u10);
}

TEST_F(UriOperator, UriBasicInequality)
{
	EXPECT_NE(Uri("http://localhost:80/path1"), Uri("https://localhost:80/path1"));
	EXPECT_NE(Uri("http://localhost:80/path1"), Uri("http://localhost2:80/path1"));
	EXPECT_NE(Uri("http://localhost:80/path1"), Uri("http://localhost:81/path1"));
	EXPECT_NE(Uri("http://localhost:80/path1"), Uri("http://localhost:80/path2"));
	EXPECT_NE(Uri("http://localhost:80/path1?key=value"), Uri("http://localhost:80/path1?key=value2"));
	EXPECT_NE(Uri("http://localhost:80/path1?key=value#nose"), Uri("http://localhost:80/path1?key=value#nose1"));
}

TEST_F(UriOperator, TestEmpty)
{
	EXPECT_EQ(Uri(), Uri());
	EXPECT_EQ(Uri("htTp://Path"), Uri("hTtp://pAth"));

	EXPECT_NE(Uri("http://path"), Uri());
	EXPECT_NE(Uri(), Uri("http://path"));
	EXPECT_NE(Uri("http://path1"), Uri("http://path2"));
}
