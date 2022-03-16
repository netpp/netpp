//
// Created by kojiko on 2022/3/16.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriSplitting : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriSplitting, SplitString)
{
	Uri uri;
	uri.setPath("/first/second/third", false);
	std::vector<utf8string> s = uri.splitPath();
	EXPECT_EQ(3u, s.size());
	EXPECT_EQ("first", s[0]);
	EXPECT_EQ("second", s[1]);
	EXPECT_EQ("third", s[2]);
}

TEST_F(UriSplitting, SplitEncodedString)
{
	Uri uri;
	uri.setPath("heh%2Ffirst/second/third", false);
	std::vector<utf8string> s = uri.splitPath();
	EXPECT_EQ(3u, s.size());
	EXPECT_EQ("heh%2Ffirst", s[0]);
	EXPECT_EQ("second", s[1]);
	EXPECT_EQ("third", s[2]);
}

TEST_F(UriSplitting, SplitNoSlash)
{
	Uri uri("noslash");
	uri.setPath("noslash", false);
	std::vector<utf8string> s = uri.splitPath();
	EXPECT_EQ(1u, s.size());
	EXPECT_EQ("noslash", s[0]);
}

TEST_F(UriSplitting, SplitQueryBasic)
{
	{
		// Separating with '&'
		Uri uri;
		uri.setQuery("key1=value1&key2=value2&key3=value3", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(3u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key1", iter->first);
		EXPECT_EQ("value1", iter->second);
		++iter;
		EXPECT_EQ("key2", iter->first);
		EXPECT_EQ("value2", iter->second);
		++iter;
		EXPECT_EQ("key3", iter->first);
		EXPECT_EQ("value3", iter->second);
	}
	{
		// Separating with ';'
		Uri uri;
		uri.setQuery("key1=value1;key2=value2;key3=value3", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(3u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key1", iter->first);
		EXPECT_EQ("value1", iter->second);
		++iter;
		EXPECT_EQ("key2", iter->first);
		EXPECT_EQ("value2", iter->second);
		++iter;
		EXPECT_EQ("key3", iter->first);
		EXPECT_EQ("value3", iter->second);
	}
}

TEST_F(UriSplitting, SplitEncodedQuery)
{
	{
		// Separating with '&'
		Uri uri;
		uri.setQuery("key=value%26key1%20=value1&key2=%5Evalue2&key3=value3%20", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(3u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key", iter->first);
		EXPECT_EQ("value%26key1%20=value1", iter->second);
		++iter;
		EXPECT_EQ("key2", iter->first);
		EXPECT_EQ("%5Evalue2", iter->second);
		++iter;
		EXPECT_EQ("key3", iter->first);
		EXPECT_EQ("value3%20", iter->second);
	}
	{
		// Separating with ';'
		Uri uri;
		uri.setQuery("key=value%26key1%20=value1;key2=%5Evalue2;key3=value3%20", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(3u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key", iter->first);
		EXPECT_EQ("value%26key1%20=value1", iter->second);
		++iter;
		EXPECT_EQ("key2", iter->first);
		EXPECT_EQ("%5Evalue2", iter->second);
		++iter;
		EXPECT_EQ("key3", iter->first);
		EXPECT_EQ("value3%20", iter->second);
	}
}

TEST_F(UriSplitting, SplitQueryEmpty)
{
	Uri uri;
	std::map<utf8string, utf8string> keyMap = uri.splitQuery();
	EXPECT_EQ(0u, keyMap.size());
}

TEST_F(UriSplitting, SplitQuerySingle)
{
	Uri uri;
	uri.setQuery("key1=44", false);
	std::map<utf8string, utf8string> keyMap = uri.splitQuery();
	EXPECT_EQ(1u, keyMap.size());
	auto iter = keyMap.begin();
	EXPECT_EQ("key1", iter->first);
	EXPECT_EQ("44", iter->second);
}

TEST_F(UriSplitting, SplitQueryNoValue)
{
	Uri uri;
	uri.setQuery("key1", false);
	std::map<utf8string, utf8string> keyMap = uri.splitQuery();
	EXPECT_EQ(0u, keyMap.size());

	uri.setQuery("key1=", false);
	keyMap = uri.splitQuery();
	EXPECT_EQ(1u, keyMap.size());
	auto iter = keyMap.begin();
	EXPECT_EQ("key1", iter->first);
	EXPECT_EQ("", iter->second);

	uri.setQuery("key1&", false);
	keyMap = uri.splitQuery();
	EXPECT_EQ(0u, keyMap.size());
}

TEST_F(UriSplitting, SplitQueryNoKey)
{
	Uri uri;
	uri.setQuery("=value1", false);
	std::map<utf8string, utf8string> keyMap = uri.splitQuery();
	EXPECT_EQ(1u, keyMap.size());
	auto iter = keyMap.begin();
	EXPECT_EQ("", iter->first);
	EXPECT_EQ("value1", iter->second);
}

TEST_F(UriSplitting, split_query_end_with_amp)
{
	{
		// Separating with '&'
		Uri uri;
		uri.setQuery("key1=44&", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(1u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key1", iter->first);
		EXPECT_EQ("44", iter->second);
	}
	{
		// Separating with ';'
		Uri uri;
		uri.setQuery("key1=44;", false);
		std::map<utf8string, utf8string> keyMap = uri.splitQuery();
		EXPECT_EQ(1u, keyMap.size());
		auto iter = keyMap.begin();
		EXPECT_EQ("key1", iter->first);
		EXPECT_EQ("44", iter->second);
	}
}
