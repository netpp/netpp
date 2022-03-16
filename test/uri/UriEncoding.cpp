//
// Created by kojiko on 2022/3/16.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriEncoding : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriEncoding, EncodeString)
{
	utf8string result = UriCodec::encode("first%second\u4e2d\u56fd");
	EXPECT_EQ("first%25second%E4%B8%AD%E5%9B%BD", result);

	result = UriCodec::encode("first%second");
	EXPECT_EQ("first%25second", result);
}

TEST_F(UriEncoding, DecodeString)
{
	utf8string result = UriCodec::decode("first%25second%E4%B8%AD%E5%9B%BD");
	EXPECT_EQ("first%second\u4e2d\u56fd", result);

	result = UriCodec::decode("first%25second");
	EXPECT_EQ("first%second", result);
}

TEST_F(UriEncoding, EncodeCharactersInResource)
{
	utf8string result = UriCodec::encode("http://path%name/%#!%");
	EXPECT_EQ("http://path%25name/%25#!%25", result);
}

// Tests trying to encode empty strings.
TEST_F(UriEncoding, EncodeDecodeEmptyStrings)
{
	// utility::string_t
	utf8string result = UriCodec::encode("");
	EXPECT_EQ("", result);
	utf8string str = UriCodec::decode(result);
	EXPECT_EQ("", str);

	// std::wstring
	result = UriCodec::encode("");
	EXPECT_EQ("", result);
	auto wstr = UriCodec::decode(result);
	EXPECT_EQ("", wstr);
}

// Tests encoding in various components of the URI.
TEST_F(UriEncoding, EncodeUriMultipleComponents)
{
// only encodes characters that aren't in the unreserved and reserved set.

	utf8string str("htt p://^localhost:80/path ?^one=two# frag");
	utf8string result = UriCodec::encode(str);
	EXPECT_EQ("htt%20p://%5Elocalhost:80/path%20?%5Eone=two#%20frag", result);
	EXPECT_EQ(str, UriCodec::decode(result));
}

// Tests encoding individual components of a URI.
TEST_F(UriEncoding, EncodeUriComponent)
{
// encodes all characters not in the unreserved set.

	utf8string str("path with^spaced");
	utf8string result = UriCodec::encode(str);
	EXPECT_EQ("path%20with%5Espaced", result);
	EXPECT_EQ(str, UriCodec::decode(result));
}

// Tests trying to decode a string that doesn't have 2 hex digits after %
TEST_F(UriEncoding, DecodeInvalidHex)
{
	EXPECT_THROW(UriCodec::decode("hehe%"), UriException);
	EXPECT_THROW(UriCodec::decode("hehe%2"), UriException);
	EXPECT_THROW(UriCodec::decode("hehe%4H"), UriException);
	EXPECT_THROW(UriCodec::decode("he%kkhe"), UriException);
}

// Tests making sure '+' is encoded even though nonstandard, so it doesn't
// get mistaken later by some implementations as a space.
TEST_F(UriEncoding, EncodePlusChar)
{
	const utf8string encodedPlus("%2B");

	Uri uri;
	uri.setUserInfo("+", true);
	uri.setPath("+", true);
	uri.setQuery("+", true);
	uri.setFragment("+", true);

	EXPECT_EQ(uri.userInfo(), encodedPlus);
	EXPECT_EQ(uri.path(), encodedPlus);
	EXPECT_EQ(uri.query(), encodedPlus);
	EXPECT_EQ(uri.fragment(), encodedPlus);
}

/*TEST_F(UriEncoding, bug_417601)
{
	std::ostringstream ss1;
	auto enc1 = uri::encode_data_string(U("!"));
	ss1 << enc1;

	VERIFY_ARE_EQUAL(U("%21"), ss1.str());
}*/