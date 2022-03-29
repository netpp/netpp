//
// Created by gaojian on 2022/3/16.
//

#include <gtest/gtest.h>
#include "uri/Uri.h"
#include "uri/UriCodec.h"
#include "uri/UriException.h"

using namespace netpp::uri;

class UriConstructor : public testing::Test {
public:
protected:
	void SetUp() override
	{}

	void TearDown() override
	{}
};

TEST_F(UriConstructor, ParsingConstructorChar)
{
	Uri u(UriCodec::encode("net.tcp://steve:@testname.com:81/bleh%?qstring#goo"));

	EXPECT_EQ("net.tcp", u.scheme());
	EXPECT_EQ("steve:", u.userInfo());
	EXPECT_EQ("testname.com", u.host());
	EXPECT_EQ(81, u.port());
	EXPECT_EQ("/bleh%25", u.path());
	EXPECT_EQ("qstring", u.query());
	EXPECT_EQ("goo", u.fragment());
}

TEST_F(UriConstructor, ParsingConstructorEncodedString)
{
	Uri u(UriCodec::encode("net.tcp://testname.com:81/bleh%?qstring#goo"));

	EXPECT_EQ("net.tcp", u.scheme());
	EXPECT_EQ("testname.com", u.host());
	EXPECT_EQ(81, u.port());
	EXPECT_EQ("/bleh%25", u.path());
	EXPECT_EQ("qstring", u.query());
	EXPECT_EQ("goo", u.fragment());
}

TEST_F(UriConstructor, ParsingConstructorStringString)
{
	Uri u(UriCodec::encode("net.tcp://testname.com:81/bleh%?qstring#goo"));

	EXPECT_EQ("net.tcp", u.scheme());
	EXPECT_EQ("testname.com", u.host());
	EXPECT_EQ(81, u.port());
	EXPECT_EQ("/bleh%25", u.path());
	EXPECT_EQ("qstring", u.query());
	EXPECT_EQ("goo", u.fragment());
}

TEST_F(UriConstructor, EmptyStrings)
{
	EXPECT_TRUE(Uri("").isEmpty());
	EXPECT_TRUE(Uri(UriCodec::encode("")).isEmpty());
}

TEST_F(UriConstructor, DefaultConstructor)
{
	EXPECT_TRUE(Uri().isEmpty());
}

TEST_F(UriConstructor, RelativeRefString)
{
	Uri u("first/second#boff");

	EXPECT_EQ("", u.scheme());
	EXPECT_EQ("", u.host());
	EXPECT_EQ(0, u.port());
	EXPECT_EQ("first/second", u.path());
	EXPECT_EQ("", u.query());
	EXPECT_EQ("boff", u.fragment());
}

TEST_F(UriConstructor, AbsoluteRefString)
{
	Uri u("/first/second#boff");

	EXPECT_EQ("", u.scheme());
	EXPECT_EQ("", u.host());
	EXPECT_EQ(0, u.port());
	EXPECT_EQ("/first/second", u.path());
	EXPECT_EQ("", u.query());
	EXPECT_EQ("boff", u.fragment());
}

TEST_F(UriConstructor, CopyConstructor)
{
	Uri original("http://st:pass@localhost:456/path1?qstring#goo");
	Uri new_uri(original);

	EXPECT_EQ(original, new_uri);
}

TEST_F(UriConstructor, MoveConstructor)
{
	const utf8string uri_str("http://localhost:456/path1?qstring#goo");
	Uri original(uri_str);
	Uri new_uri = std::move(original);

	EXPECT_EQ(uri_str, new_uri.toString());
	EXPECT_EQ(Uri(uri_str), new_uri);
}

TEST_F(UriConstructor, AssignmentOperator)
{
	Uri original("http://localhost:456/path?qstring#goo");
	Uri new_uri = original;

	EXPECT_EQ(original, new_uri);
}

// Tests invalid URI being passed in constructor.
TEST_F(UriConstructor, ParsingConstructorInvalid)
{
	EXPECT_THROW(Uri("123http://localhost:345/"), UriException);
	EXPECT_THROW(Uri("h*ttp://localhost:345/"), UriException);
	EXPECT_THROW(Uri("http://localhost:345/\""), UriException);
	EXPECT_THROW(Uri("http://localhost:345/path?\""), UriException);
	EXPECT_THROW(Uri("http://local\"host:345/"), UriException);
}

// Tests a variety of different URIs using the examples in RFC 2732
TEST_F(UriConstructor, RFC_2732_examples_string)
{
	// The URI parser will make characters lower case
	Uri http1("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html");
	EXPECT_EQ("http", http1.scheme());
	EXPECT_EQ("[fedc:ba98:7654:3210:fedc:ba98:7654:3210]", http1.host());
	EXPECT_EQ(80, http1.port());
	EXPECT_EQ("/index.html", http1.path());
	EXPECT_EQ("", http1.query());

	Uri http2("http://[1080:0:0:0:8:800:200C:417A]/index.html");
	EXPECT_EQ("http", http2.scheme());
	EXPECT_EQ("[1080:0:0:0:8:800:200c:417a]", http2.host());
	EXPECT_EQ(0, http2.port());
	EXPECT_EQ("/index.html", http2.path());
	EXPECT_EQ("", http2.query());

	Uri http3("https://[3ffe:2a00:100:7031::1]");
	EXPECT_EQ("https", http3.scheme());
	EXPECT_EQ("[3ffe:2a00:100:7031::1]", http3.host());
	EXPECT_EQ(0, http3.port());
	EXPECT_EQ("/", http3.path());
	EXPECT_EQ("", http3.query());

	Uri http4("http://[::192.9.5.5]/ipng");
	EXPECT_EQ("http", http4.scheme());
	EXPECT_EQ("[::192.9.5.5]", http4.host());
	EXPECT_EQ(0, http4.port());
	EXPECT_EQ("/ipng", http4.path());
	EXPECT_EQ("", http4.query());

	Uri http5("http://[1080::8:800:200C:417A]/foo");
	EXPECT_EQ("http", http5.scheme());
	EXPECT_EQ("[1080::8:800:200c:417a]", http5.host());
	EXPECT_EQ(0, http5.port());
	EXPECT_EQ("/foo", http5.path());
	EXPECT_EQ("", http5.query());

	Uri http6("http://[::FFFF:129.144.52.38]:80/index.html");
	EXPECT_EQ("http", http6.scheme());
	EXPECT_EQ("[::ffff:129.144.52.38]", http6.host());
	EXPECT_EQ(80, http6.port());
	EXPECT_EQ("/index.html", http6.path());
	EXPECT_EQ("", http6.query());

	Uri http7("http://[2010:836B:4179::836B:4179]");
	EXPECT_EQ("http", http7.scheme());
	EXPECT_EQ("[2010:836b:4179::836b:4179]", http7.host());
	EXPECT_EQ(0, http7.port());
	EXPECT_EQ("/", http7.path());
	EXPECT_EQ("", http7.query());
}

// Tests a variety of different URIs using the examples in RFC 3986.
TEST_F(UriConstructor, RFC_3968_examples_string)
{
	Uri ftp("ftp://ftp.is.co.za/rfc/rfc1808.txt");
	EXPECT_EQ("ftp", ftp.scheme());
	EXPECT_EQ("", ftp.userInfo());
	EXPECT_EQ("ftp.is.co.za", ftp.host());
	EXPECT_EQ(0, ftp.port());
	EXPECT_EQ("/rfc/rfc1808.txt", ftp.path());
	EXPECT_EQ("", ftp.query());
	EXPECT_EQ("", ftp.fragment());

	// TFS #371892
	// uri ldap(U("ldap://[2001:db8::7]/?c=GB#objectClass?one"));
	// VERIFY_ARE_EQUAL(U("ldap"), ldap.scheme());
	// VERIFY_ARE_EQUAL(U(""), ldap.user_info());
	// VERIFY_ARE_EQUAL(U("2001:db8::7"), ldap.host());
	// VERIFY_ARE_EQUAL(0, ldap.port());
	// VERIFY_ARE_EQUAL(U("/"), ldap.path());
	// VERIFY_ARE_EQUAL(U("c=GB"), ldap.query());
	// VERIFY_ARE_EQUAL(U("objectClass?one"), ldap.fragment());

	// We don't support anything scheme specific like in C# so
	// these common ones don't have a great experience yet.
	Uri mailto("mailto:John.Doe@example.com");
	EXPECT_EQ("mailto", mailto.scheme());
	EXPECT_EQ("", mailto.userInfo());
	EXPECT_EQ("", mailto.host());
	EXPECT_EQ(0, mailto.port());
	EXPECT_EQ("John.Doe@example.com", mailto.path());
	EXPECT_EQ("", mailto.query());
	EXPECT_EQ("", mailto.fragment());

	Uri tel("tel:+1-816-555-1212");
	EXPECT_EQ("tel", tel.scheme());
	EXPECT_EQ("", tel.userInfo());
	EXPECT_EQ("", tel.host());
	EXPECT_EQ(0, tel.port());
	EXPECT_EQ("+1-816-555-1212", tel.path());
	EXPECT_EQ("", tel.query());
	EXPECT_EQ("", tel.fragment());

	Uri telnet("telnet://192.0.2.16:80/");
	EXPECT_EQ("telnet", telnet.scheme());
	EXPECT_EQ("", telnet.userInfo());
	EXPECT_EQ("192.0.2.16", telnet.host());
	EXPECT_EQ(80, telnet.port());
	EXPECT_EQ("/", telnet.path());
	EXPECT_EQ("", telnet.query());
	EXPECT_EQ("", telnet.fragment());
}

TEST_F(UriConstructor, user_info_string)
{
	Uri ftp("ftp://johndoe:testname@ftp.is.co.za/rfc/rfc1808.txt");
	EXPECT_EQ("ftp", ftp.scheme());
	EXPECT_EQ("johndoe:testname", ftp.userInfo());
	EXPECT_EQ("ftp.is.co.za", ftp.host());
	EXPECT_EQ(0, ftp.port());
	EXPECT_EQ("/rfc/rfc1808.txt", ftp.path());
	EXPECT_EQ("", ftp.query());
	EXPECT_EQ("", ftp.fragment());
}

// Test query component can be separated with '&' or ';'.
TEST_F(UriConstructor, query_seperated_with_semi_colon)
{
	Uri u("http://localhost/path1?key1=val1;key2=val2");
	EXPECT_EQ("key1=val1;key2=val2", u.query());
}