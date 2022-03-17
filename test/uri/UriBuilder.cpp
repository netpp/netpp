//
// Created by kojiko on 2022/3/17.
//

#include <gtest/gtest.h>
#include "http/Uri.h"

using namespace netpp::http;

class UriBuilder : public testing::Test {
public:
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}

	// Helper functions to verify components of a builder.
	void VERIFY_URI_BUILDER(Uri &uri,
							const utf8string &scheme,
							const utf8string &user_info,
							const utf8string &host,
							const int port,
							const utf8string &path,
							const utf8string &query,
							const utf8string &fragment)
	{
		EXPECT_EQ(scheme, uri.scheme());
		EXPECT_EQ(host, uri.host());
		EXPECT_EQ(user_info, uri.userInfo());
		EXPECT_EQ(port, uri.port());
		EXPECT_EQ(path, uri.path());
		EXPECT_EQ(query, uri.query());
		EXPECT_EQ(fragment, uri.fragment());
	}

	void VERIFY_URI_BUILDER(Uri &uri,
							const utf8string &scheme,
							const utf8string &host,
							const int port)
	{
		VERIFY_URI_BUILDER(uri,
						   scheme,
						   "",
						   host,
						   port,
						   "/",
						   "",
						   "");
	}

	void VERIFY_URI_BUILDER_IS_EMPTY(Uri &uri)
	{
		VERIFY_URI_BUILDER(uri,
						   "",
						   "",
						   "",
						   0,
						   "/",
						   "",
						   "");
	}
};

TEST_F(UriBuilder, Constructor)
{
	// Default constructor
	Uri uri;
	VERIFY_URI_BUILDER_IS_EMPTY(uri);
	// scheme, user_info, host, port
	utf8string scheme("ftp");
	utf8string user_info("steve:pass");
	utf8string host("localhost");
	int port = 44;
	utf8string path("/Yeshere888");
	utf8string uri_str("ftp://steve:pass@localhost:44/Yeshere888");

	utf8string uri_wstr("ftp://steve:pass@localhost:44/Yeshere888?abc:123&abc2:456#nose");
	uri = Uri(uri_wstr);
	VERIFY_URI_BUILDER(uri,
					   scheme,
					   user_info,
					   host,
					   port,
					   path,
					   "abc:123&abc2:456",
					   "nose");

	// copy constructor
	Uri other(uri);
	uri = Uri(uri_str);
	VERIFY_URI_BUILDER(other,
					   scheme,
					   user_info,
					   host,
					   port,
					   path,
					   "abc:123&abc2:456",
					   "nose");
	VERIFY_URI_BUILDER(uri, scheme, user_info, host, port, path, "", "");

	// move constructor
	Uri move_other = std::move(uri);
	VERIFY_URI_BUILDER(move_other, scheme, user_info, host, port, path, "", "");
}

TEST_F(UriBuilder, AssignmentOperators)
{
	// assignment operator
	const utf8string scheme = "http", host = "localhost";
	const int port = 44;
	Uri original;
	original.setScheme(scheme).setHost(host).setPort(port);
	Uri assign;
	assign = original;
	VERIFY_URI_BUILDER(assign, scheme, host, port);

	// move assignment operator
	Uri move_assign;
	move_assign = std::move(original);
	VERIFY_URI_BUILDER(assign, scheme, host, port);
}

TEST_F(UriBuilder, SetPort)
{
	Uri uri;

	EXPECT_EQ(0, uri.port());

	uri.setPort(987);
	EXPECT_EQ(987, uri.port());
}

TEST_F(UriBuilder, ComponentAssignment)
{
	Uri uri;
	const utf8string scheme("myscheme");
	const utf8string uinfo("johndoe:test");
	const utf8string host("localhost");
	const int port = 88;
	const utf8string path("jklajsd");
	const utf8string query("key1=val1");
	const utf8string fragment("last");

	uri.setScheme(scheme);
	uri.setUserInfo(uinfo);
	uri.setHost(host);
	uri.setPort(port);
	uri.setPath(path);
	uri.setQuery(query);
	uri.setFragment(fragment);

	VERIFY_URI_BUILDER(uri, scheme, uinfo, host, port, "/" + path, query, fragment);
}

TEST_F(UriBuilder, ComponentAssignmentEncode)
{
	{
		Uri uri;
		const utf8string scheme("myscheme");
		const utf8string uinfo("johndoe:test");
		const utf8string host("localhost");
		const int port = 88;
		const utf8string path("jklajsd/yes no");
		const utf8string query("key1=va%l1");
		const utf8string fragment("las t");

		uri.setScheme(scheme);
		uri.setUserInfo(uinfo, true);
		uri.setHost(host, true);
		uri.setPort(port);
		uri.setPath(path, true);
		uri.setQuery(query, true);
		uri.setFragment(fragment, true);

		VERIFY_URI_BUILDER(uri,
						   scheme,
						   "johndoe:test",
						   "localhost",
						   port,
						   "/jklajsd/yes%20no",
						   "key1=va%25l1",
						   "las%20t");
	}
	{
		Uri uri;
		const utf8string scheme("myscheme");
		const utf8string uinfo("johndoe:test");
		const utf8string host("localhost");
		const int port = 88;
		const utf8string path("jklajsd/yes no");
		const utf8string query("key1=va%l1");
		const utf8string fragment("las t");

		uri.setScheme(scheme);
		uri.setUserInfo(uinfo, true);
		uri.setHost(host, true);
		uri.setPort(port);
		uri.setPath(path, true);
		uri.setQuery(query, true);
		uri.setFragment(fragment, true);

		VERIFY_URI_BUILDER(uri,
						   scheme,
						   "johndoe:test",
						   "localhost",
						   port,
						   "/jklajsd/yes%20no",
						   "key1=va%25l1",
						   "las%20t");
	}
}

TEST_F(UriBuilder, Validation)
{
	// true
	Uri uri("http://localhost:4567/");
	EXPECT_TRUE(uri.validate());

	// false
	uri = Uri();
	uri.setScheme("123");
	EXPECT_FALSE(uri.validate());
}

TEST_F(UriBuilder, UriCreationString)
{
	utf8string uri_str("http://steve:temp@localhost:4556/");

	// to_string
	Uri uri(uri_str);
	EXPECT_EQ(uri_str, uri.toString());

	// to encoded string
	Uri with_space(uri_str);
	with_space.setPath("path%20with%20space", false);
	EXPECT_EQ("http://steve:temp@localhost:4556/path%20with%20space", with_space.toString());
}

TEST_F(UriBuilder, AppendPathString)
{
	// empty uri builder path
	Uri uri;
	uri.appendPath("/path1");
	EXPECT_EQ("/path1", uri.path());

	// empty append path
	uri.appendPath("");
	EXPECT_EQ("/path1", uri.path());

	// uri builder with slash
	uri.appendPath("/");
	uri.appendPath("path2");
	EXPECT_EQ("/path1/path2", uri.path());

	// both with slash
	uri.appendPath("/");
	uri.appendPath("/path3");
	EXPECT_EQ("/path1/path2/path3", uri.path());

	// both without slash
	uri.appendPath("path4");
	EXPECT_EQ("/path1/path2/path3/path4", uri.path());

	{
		// encoding
		Uri uri1;
		uri1.appendPath("encode%things", false);
		EXPECT_EQ("/encode%things", uri1.path());
	}

	{
		Uri uri1;
		uri1.appendPath("encode%things", false);
		EXPECT_EQ("/encode%things", uri1.path());
	}

	{
		Uri uri1;
		uri1.appendPath("encode%things", true);
		EXPECT_EQ("/encode%25things", uri1.path());
	}

	// self references
	uri.setPath("example");
	uri.appendPath(uri.path());
	EXPECT_EQ("/example/example", uri.path());

	uri.setPath("/example");
	uri.appendPath(uri.path());
	EXPECT_EQ("/example/example", uri.path());

	uri.setPath("/example/");
	uri.appendPath(uri.path());
	EXPECT_EQ("/example/example/", uri.path());
}

TEST_F(UriBuilder, AppendQueryString)
{
	// empty uri builder query
	Uri uri;
	uri.appendQuery("key1=value1");
	EXPECT_EQ("key1=value1", uri.query());

	// empty append query
	uri.appendQuery("");
	EXPECT_EQ("key1=value1", uri.query());

	// uri builder with ampersand
	uri.appendQuery("&");
	uri.appendQuery("key2=value2");
	EXPECT_EQ("key1=value1&key2=value2", uri.query());

	// both with ampersand
	uri.appendQuery("&");
	uri.appendQuery("&key3=value3");
	EXPECT_EQ("key1=value1&key2=value2&key3=value3", uri.query());

	// both without ampersand
	uri.appendQuery("key4=value4");
	EXPECT_EQ("key1=value1&key2=value2&key3=value3&key4=value4", uri.query());

	// number query
	uri.appendQuery("key5", 1);
	EXPECT_EQ("key1=value1&key2=value2&key3=value3&key4=value4&key5=1", uri.query());

	// string query
	uri.appendQuery("key6", "val6");
	EXPECT_EQ("key1=value1&key2=value2&key3=value3&key4=value4&key5=1&key6=val6", uri.query());

	// key and value separate with '=', '&', and ';'
	uri.appendQuery("key=&;", "=&;value");
	EXPECT_EQ(
			"key1=value1&key2=value2&key3=value3&key4=value4&key5=1&key6=val6&key%3D%26%3B=%3D%26%3Bvalue",
			uri.query());

	// self references
	uri.setQuery("example");
	uri.appendQuery(uri.query());
	EXPECT_EQ("example&example", uri.query());

	uri.setQuery("&example");
	uri.appendQuery(uri.query());
	EXPECT_EQ("&example&example", uri.query());

	uri.setQuery("&example&");
	uri.appendQuery(uri.query());
	EXPECT_EQ("&example&example&", uri.query());
}

TEST_F(UriBuilder, AppendQueryStringNoEncode)
{
	Uri uri;
	uri.appendQuery("key=&;", "=&;value", false);
	EXPECT_EQ("key=&;==&;value", uri.query());
}

TEST_F(UriBuilder, AppendPathEncoding)
{
	Uri uri;
	uri.appendPath("/path space", true);
	EXPECT_EQ("/path%20space", uri.path());

	uri.appendPath("path2");
	EXPECT_EQ("/path%20space/path2", uri.path());
}

TEST_F(UriBuilder, AppendQueryEncoding)
{
	Uri uri;
	uri.appendQuery("key1 =value2", true);
	EXPECT_EQ("key1%20=value2", uri.query());

	uri.appendQuery("key2=value3");
	EXPECT_EQ("key1%20=value2&key2=value3", uri.query());
}

TEST_F(UriBuilder, HostEncoding)
{
	// Check that ASCII characters that are invalid in a host name
	// do not get percent-encoded.

	Uri uri;
	uri.setScheme("http").setHost("????dfasddsf!@#$%^&*()_+").setPort(80);

	EXPECT_FALSE(uri.validate());
}

/*#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
TEST_F(UriBuilder, ToStringInvalidUri)
{
	Uri uri("http://invaliduri.com");
	uri.setScheme("1http");
	EXPECT_THROW(uri.toString(), UriException);

	uri.setScheme("ht*ip");
	EXPECT_THROW(uri.toString(), UriException);

	uri.setScheme("htt%20p");
	EXPECT_THROW(uri.toString(), UriException);
}
#pragma GCC diagnostic pop*/

TEST_F(UriBuilder, AppendQueryLocale)
{
	std::locale changedLocale;
	try
	{
		changedLocale = std::locale("fr_FR.UTF-8");
	}
	catch (const std::exception &)
	{
		// Silently pass if locale isn't installed on machine.
		return;
	}

	std::locale prev = std::locale::global(changedLocale);

	Uri uri;
	auto const &key = "key1000";
	uri.appendQuery(key, 1000);
	utf8string expected(key);
	expected.append("=1000");
	EXPECT_EQ(expected, uri.query());

	std::locale::global(prev);
}

TEST_F(UriBuilder, github_crash_994)
{
	Uri uri("http://127.0.0.1:34568/");
}
