#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <charconv>
#include <functional>
#include "uri/Types.h"
#include "uri/Uri.h"
#include "uri/UriCodec.h"
#include "uri/UriException.h"

using namespace netpp::uri;

/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * Protocol independent support for URIs.
 *
 * For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

namespace details {
/// <summary>
/// Unreserved characters are those that are allowed in a URI but do not have a reserved purpose. They include:
/// - A-Z
/// - a-z
/// - 0-9
/// - '-' (hyphen)
/// - '.' (period)
/// - '_' (underscore)
/// - '~' (tilde)
/// </summary>
inline bool is_unreserved(int c)
{
	return std::isalnum((char) c) || c == '-' || c == '.' || c == '_' || c == '~';
}

/// <summary>
/// General delimiters serve as the delimiters between different uri components.
/// General delimiters include:
/// - All of these :/?#[]@
/// </summary>
inline bool is_gen_delim(int c)
{
	return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@';
}

/// <summary>
/// Subdelimiters are those characters that may have a defined meaning within component
/// of a uri for a particular scheme. They do not serve as delimiters in any case between
/// uri segments. sub_delimiters include:
/// - All of these !$&'()*+,;=
/// </summary>
inline bool is_sub_delim(int c)
{
	switch (c)
	{
		case '!':
		case '$':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case ';':
		case '=': return true;
		default: return false;
	}
}

/// <summary>
/// Reserved characters includes the general delimiters and sub delimiters. Some characters
/// are neither reserved nor unreserved, and must be percent-encoded.
/// </summary>
inline bool is_reserved(int c)
{ return is_gen_delim(c) || is_sub_delim(c); }

/// <summary>
/// Legal characters in the scheme portion include:
/// - Any alphanumeric character
/// - '+' (plus)
/// - '-' (hyphen)
/// - '.' (period)
///
/// Note that the scheme must BEGIN with an alpha character.
/// </summary>
inline bool is_scheme_character(int c)
{
	return std::isalnum((char) c) || c == '+' || c == '-' || c == '.';
}

/// <summary>
/// Legal characters in the user information portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// </summary>
inline bool is_user_info_character(int c)
{ return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == ':'; }

/// <summary>
/// Legal characters in the authority portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// - IPv6 requires '[]' allowed for it to be valid URI and passed to underlying platform for IPv6 support
/// </summary>
inline bool is_authority_character(int c)
{
	return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == '@' || c == ':' || c == '[' || c == ']';
}

/// <summary>
/// Legal characters in the path portion include:
/// - Any unreserved character
/// - The percent character ('%'), and thus any percent-endcoded octet
/// - The sub-delimiters
/// - ':' (colon)
/// - '@' (at sign)
/// </summary>
inline bool is_path_character(int c)
{
	return is_unreserved(c) || is_sub_delim(c) || c == '%' || c == '/' || c == ':' || c == '@';
}

/// <summary>
/// Legal characters in the query portion include:
/// - Any path character
/// - '?' (question mark)
/// </summary>
inline bool is_query_character(int c)
{ return is_path_character(c) || c == '?'; }

/// <summary>
/// Legal characters in the fragment portion include:
/// - Any path character
/// - '?' (question mark)
/// </summary>
inline bool is_fragment_character(int c)
{
	// this is intentional, they have the same set of legal characters
	return is_query_character(c);
}

struct inner_parse_out {
	const char *scheme_begin = nullptr;
	const char *scheme_end = nullptr;
	const char *uinfo_begin = nullptr;
	const char *uinfo_end = nullptr;
	const char *host_begin = nullptr;
	const char *host_end = nullptr;
	int port = 0;
	const char *path_begin = nullptr;
	const char *path_end = nullptr;
	const char *query_begin = nullptr;
	const char *query_end = nullptr;
	const char *fragment_begin = nullptr;
	const char *fragment_end = nullptr;

	/// <summary>
	/// Parses the uri, setting the given pointers to locations inside the given buffer.
	/// 'encoded' is expected to point to an encoded zero-terminated string containing a uri
	/// </summary>
	bool parse_from(const char *encoded)
	{
		const char *p = encoded;

		// IMPORTANT -- A uri may either be an absolute uri, or an relative-reference
		// Absolute: 'http://host.com'
		// Relative-Reference: '//:host.com', '/path1/path2?query', './path1:path2'
		// A Relative-Reference can be disambiguated by parsing for a ':' before the first slash

		bool is_relative_reference = true;
		const char *p2 = p;
		for (; *p2 != '/' && *p2 != '\0'; p2++)
		{
			if (*p2 == ':')
			{
				// found a colon, the first portion is a scheme
				is_relative_reference = false;
				break;
			}
		}

		if (!is_relative_reference)
		{
			// the first character of a scheme must be a letter
			if (!isalpha(*p))
			{
				return false;
			}

			// start parsing the scheme, it's always delimited by a colon (must be present)
			scheme_begin = p++;
			for (; *p != ':'; p++)
			{
				if (!is_scheme_character(*p))
				{
					return false;
				}
			}
			scheme_end = p;

			// skip over the colon
			p++;
		}

		// if we see two slashes next, then we're going to parse the authority portion
		// later on we'll break up the authority into the port and host
		const char *authority_begin = nullptr;
		const char *authority_end = nullptr;
		if (*p == '/' && p[1] == '/')
		{
			// skip over the slashes
			p += 2;
			authority_begin = p;

			// the authority is delimited by a slash (resource), question-mark (query) or octothorpe (fragment)
			// or by EOS. The authority could be empty ('file:///C:\file_name.txt')
			for (; *p != '/' && *p != '?' && *p != '#' && *p != '\0'; p++)
			{
				// We're NOT currently supporting IPvFuture or username/password in authority
				// IPv6 as the host (i.e. http://[:::::::]) is allowed as valid URI and passed to subsystem for support.
				if (!is_authority_character(*p))
				{
					return false;
				}
			}
			authority_end = p;

			// now lets see if we have a port specified -- by working back from the end
			if (authority_begin != authority_end)
			{
				// the port is made up of all digits
				const char *port_begin = authority_end - 1;
				for (; isdigit(*port_begin) && port_begin != authority_begin; port_begin--)
				{
				}

				if (*port_begin == ':')
				{
					// has a port
					host_begin = authority_begin;
					host_end = port_begin;

					// skip the colon
					port_begin++;

					std::from_chars(port_begin, authority_end, port);
				}
				else
				{
					// no port
					host_begin = authority_begin;
					host_end = authority_end;
				}

				// look for a user_info component
				const char *u_end = host_begin;
				for (; is_user_info_character(*u_end) && u_end != host_end; u_end++)
				{
				}

				if (*u_end == '@')
				{
					host_begin = u_end + 1;
					uinfo_begin = authority_begin;
					uinfo_end = u_end;
				}
			}
		}

		// if we see a path character or a slash, then the
		// if we see a slash, or any other legal path character, parse the path next
		if (*p == '/' || is_path_character(*p))
		{
			path_begin = p;

			// the path is delimited by a question-mark (query) or octothorpe (fragment) or by EOS
			for (; *p != '?' && *p != '#' && *p != '\0'; p++)
			{
				if (!is_path_character(*p))
				{
					return false;
				}
			}
			path_end = p;
		}

		// if we see a ?, then the query is next
		if (*p == '?')
		{
			// skip over the question mark
			p++;
			query_begin = p;

			// the query is delimited by a '#' (fragment) or EOS
			for (; *p != '#' && *p != '\0'; p++)
			{
				if (!is_query_character(*p))
				{
					return false;
				}
			}
			query_end = p;
		}

		// if we see a #, then the fragment is next
		if (*p == '#')
		{
			// skip over the hash mark
			p++;
			fragment_begin = p;

			// the fragment is delimited by EOS
			for (; *p != '\0'; p++)
			{
				if (!is_fragment_character(*p))
				{
					return false;
				}
			}
			fragment_end = p;
		}

		return true;
	}
};

// Encodes all characters not in given set determined by given function.
utf8string encode_impl(const utf8string &raw, const std::function<bool(int)> &should_encode)
{
	const char *const hex = "0123456789ABCDEF";
	utf8string encoded;
	for (char iter: raw)
	{
		// for utf8 encoded string, char ASCII can be greater than 127.
		int ch = static_cast<unsigned char>(iter);
		// ch should be same under both utf8 and utf16.
		if (should_encode(ch))
		{
			encoded.push_back('%');
			encoded.push_back(hex[(ch >> 4) & 0xF]);
			encoded.push_back(hex[ch & 0xF]);
		}
		else
		{
			// ASCII don't need to be encoded, which should be same on both utf8 and utf16.
			encoded.push_back((char) ch);
		}
	}
	return encoded;
}

// 5.2.3. Merge Paths https://tools.ietf.org/html/rfc3986#section-5.2.3
utf8string mergePaths(const utf8string &base, const utf8string &relative)
{
	const auto lastSlash = base.rfind('/');
	if (lastSlash == utf8string::npos)
	{
		return base + '/' + relative;
	}
	else if (lastSlash == base.size() - 1)
	{
		return base + relative;
	}
	// path contains and does not end with '/', we remove segment after last '/'
	return base.substr(0, lastSlash + 1) + relative;
}

// 5.2.4. Remove Dot Segments https://tools.ietf.org/html/rfc3986#section-5.2.4
void removeDotSegments(Uri &uri)
{
	static const utf8string dotSegment(".");
	static const utf8string dotDotSegment("..");

	if (uri.path().find('.') == utf8string::npos) return;

	const auto segments = uri.splitPath();
	std::vector<std::reference_wrapper<const utf8string>> result;
	for (auto &segment: segments)
	{
		if (segment == dotSegment)
			continue;
		else if (segment != dotDotSegment)
			result.emplace_back(segment);
		else if (!result.empty())
			result.pop_back();
	}
	if (result.empty())
	{
		uri.setPath(utf8string());
		return;
	}
	utf8string path = result.front().get();
	for (size_t i = 1; i != result.size(); ++i)
	{
		path += '/';
		path += result[i].get();
	}
	if (segments.back() == dotDotSegment || segments.back() == dotSegment || uri.path().back() == '/')
	{
		path += '/';
	}

	uri.setPath(path);
}

char to_lower_ch_impl(char c)
{
	if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
	return c;
}

void inplace_tolower(std::string &target) noexcept
{
	for (auto &ch: target)
	{
		ch = to_lower_ch_impl(ch);
	}
}

/// <summary>
/// Helper function to convert a hex character digit to a decimal character value.
/// Throws an exception if not a valid hex digit.
/// </summary>
static int hex_char_digit_to_decimal_char(int hex)
{
	int decimal;
	if (hex >= '0' && hex <= '9')
	{
		decimal = hex - '0';
	}
	else if (hex >= 'A' && hex <= 'F')
	{
		decimal = 10 + (hex - 'A');
	}
	else if (hex >= 'a' && hex <= 'f')
	{
		decimal = 10 + (hex - 'a');
	}
	else
	{
		throw UriException("Invalid hexadecimal digit");
	}
	return decimal;
}
}

namespace netpp::uri {
Uri::Uri()
		: Uri("/")
{}

Uri::Uri(const char *uri)
{
	details::inner_parse_out out;

	if (!out.parse_from(uri))
	{
		throw UriException("provided uri is invalid: " + utf8string(uri));
	}

	if (out.scheme_begin)
	{
		m_scheme.assign(out.scheme_begin, out.scheme_end);
		details::inplace_tolower(m_scheme);
	}

	if (out.uinfo_begin)
		m_user_info.assign(out.uinfo_begin, out.uinfo_end);

	if (out.host_begin)
	{
		m_host.assign(out.host_begin, out.host_end);
		details::inplace_tolower(m_host);
	}

	m_port = out.port;

	if (out.path_begin)
		m_path.assign(out.path_begin, out.path_end);
	else    // default path to begin with a slash for easy comparison
		m_path = "/";

	if (out.query_begin)
		m_query.assign(out.query_begin, out.query_end);

	if (out.fragment_begin)
		m_fragment.assign(out.fragment_begin, out.fragment_end);
}

Uri::Uri(const utf8string &uri)
		: Uri(uri.c_str())
{}

Uri &Uri::setHost(const utf8string &host, bool shouldEncode)
{
	if (shouldEncode)
		m_host = UriCodec::encode(host, UriCodec::UriComponent::host);
	else
		m_host = host;

	return *this;
}

Uri &Uri::setUserInfo(const utf8string &userInfo, bool shouldEncode)
{
	if (shouldEncode)
		m_user_info = UriCodec::encode(userInfo, UriCodec::UriComponent::userInfo);
	else
		m_user_info = userInfo;

	return *this;
}

Uri &Uri::setPath(const utf8string &path, bool shouldEncode)
{
	utf8string prepend;
	if (path.empty() || (!path.empty() && path[0] != '/'))
		prepend = "/";
	if (shouldEncode)
		m_path = prepend + UriCodec::encode(path, UriCodec::UriComponent::path);
	else
		m_path = prepend + path;

	return *this;
}

Uri &Uri::appendPath(const utf8string &path, bool shouldEncode)
{
	if (!path.empty() && path != "/")
	{
		auto &thisPath = m_path;
		if (&thisPath == &path)
		{
			auto appendCopy = path;
			return appendPath(appendCopy, shouldEncode);
		}

		if (thisPath.empty() || thisPath == "/")
		{
			thisPath.clear();
			if (path.front() != '/')
				thisPath.push_back('/');
		}
		else if (thisPath.back() == ('/') && path.front() == '/')
			thisPath.pop_back();
		else if (thisPath.back() != '/' && path.front() != '/')
			thisPath.push_back('/');

		if (shouldEncode)
			thisPath.append(UriCodec::encode(path, UriCodec::UriComponent::path));
		else
			thisPath.append(path);
	}

	return *this;
}

Uri &Uri::setQuery(const utf8string &query, bool shouldEncode)
{
	if (shouldEncode)
		m_query = UriCodec::encode(query, UriCodec::UriComponent::query);
	else
		m_query = query;

	return *this;
}

Uri &Uri::appendQuery(const utf8string &query, bool shouldEncode)
{
	if (!query.empty())
	{
		auto &thisQuery = m_query;
		if (&thisQuery == &query)
		{
			auto appendCopy = query;
			return appendQuery(appendCopy, shouldEncode);
		}

		if (thisQuery.empty())
			thisQuery.clear();
		else if (thisQuery.back() == '&' && query.front() == '&')
			thisQuery.pop_back();
		else if (thisQuery.back() != '&' && query.front() != '&')
			thisQuery.push_back('&');

		if (shouldEncode)
			thisQuery.append(UriCodec::encode(query, UriCodec::UriComponent::query));
		else
			thisQuery.append(query);
	}

	return *this;
}

Uri &Uri::setFragment(const utf8string &fragment, bool shouldEncode)
{
	if (shouldEncode)
		m_fragment = UriCodec::encode(fragment, UriCodec::UriComponent::query);
	else
		m_fragment = fragment;

	return *this;
}

// resolving URI according to RFC3986, Section 5 https://tools.ietf.org/html/rfc3986#section-5
Uri Uri::resolveUri(const utf8string &relativeUri) const
{
	if (relativeUri.empty())
	{
		return *this;
	}

	if (relativeUri[0] == '/') // starts with '/'
	{
		if (relativeUri.size() >= 2 && relativeUri[1] == '/') // starts with '//'
		{
			return Uri(m_scheme + ':' + relativeUri);
		}

		// otherwise relative to root
		Uri relative(relativeUri);
		Uri authorityPart = authority();
		authorityPart.appendPath(relative.m_path);
		authorityPart.appendQuery(relative.m_query);
		authorityPart.setFragment(m_fragment + relative.m_fragment);
		details::removeDotSegments(authorityPart);
		return authorityPart;
	}

	Uri url(relativeUri);
	if (!url.scheme().empty()) return url;

	if (!url.authority().isEmpty())
	{
		return url.setScheme(m_scheme);
	}

	// relative url
	Uri thisUri = *this;
	if (url.path() == "/" || url.path().empty()) // web::uri considers empty path as '/'
	{
		if (!url.query().empty())
		{
			thisUri.setQuery(url.query());
		}
	}
	else if (!this->path().empty())
	{
		thisUri.setPath(details::mergePaths(this->path(), url.path()));
		details::removeDotSegments(thisUri);
		thisUri.setQuery(url.query());
	}
	return thisUri.setFragment(url.fragment());
}

utf8string Uri::toString()
{
	// canonicalize components first

	// convert scheme to lowercase
	details::inplace_tolower(m_scheme);
	// convert host to lowercase
	details::inplace_tolower(m_host);

	// canonicalize the path to have a leading slash if it's a full uri
	if (!m_host.empty() && m_path.empty())
		m_path = "/";
	else if (!m_host.empty() && m_path[0] != '/')
		m_path.insert(m_path.begin(), 1, '/');

	utf8string ret;

	if (!m_scheme.empty())
	{
		ret.append(m_scheme);
		ret.push_back(':');
	}

	if (!m_host.empty())
	{
		ret.append("//");
		if (!m_user_info.empty())
			ret.append(m_user_info).append({'@'});
		ret.append(m_host);
		if (m_port > 0)
			ret.append({':'}).append(std::to_string(m_port));
	}

	if (!m_path.empty())
	{
		// only add the leading slash when the host is present
		if (!m_host.empty() && m_path.front() != '/')
			ret.push_back('/');
		ret.append(m_path);
	}

	if (!m_query.empty())
	{
		ret.push_back('?');
		ret.append(m_query);
	}

	if (!m_fragment.empty())
	{
		ret.push_back('#');
		ret.append(m_fragment);
	}

	return ret;
}

std::vector<utf8string> Uri::splitPath()
{
	std::vector<utf8string> results;
	std::istringstream iss(m_path);
	iss.imbue(std::locale::classic());
	utf8string s;

	while (std::getline(iss, s, '/'))
	{
		if (!s.empty())
		{
			results.push_back(s);
		}
	}

	return results;
}

std::map<utf8string, utf8string> Uri::splitQuery()
{
	std::map<utf8string, utf8string> results;

	// Split into key value pairs separated by '&'.
	size_t prev_amp_index = 0;
	while (prev_amp_index != utf8string::npos)
	{
		size_t amp_index = m_query.find_first_of('&', prev_amp_index);
		if (amp_index == utf8string::npos) amp_index = m_query.find_first_of(';', prev_amp_index);

		utf8string key_value_pair = m_query.substr(
				prev_amp_index,
				amp_index == utf8string::npos ? m_query.size() - prev_amp_index : amp_index - prev_amp_index
		);
		prev_amp_index = amp_index == utf8string::npos ? utf8string::npos : amp_index + 1;

		size_t equals_index = key_value_pair.find_first_of('=');
		if (equals_index == utf8string::npos)
		{
			continue;
		}
		else if (equals_index == 0)
		{
			utf8string value(key_value_pair.begin() + static_cast<long>(equals_index) + 1, key_value_pair.end());
			results[utf8string{}] = value;
		}
		else
		{
			utf8string key(key_value_pair.begin(), key_value_pair.begin() + static_cast<long>(equals_index));
			utf8string value(key_value_pair.begin() + static_cast<long>(equals_index) + 1, key_value_pair.end());
			results[key] = value;
		}
	}

	return results;
}

Uri Uri::authority() const
{
	Uri uri;
	uri.m_scheme = m_scheme;
	uri.m_host = m_host;
	uri.m_user_info = m_user_info;
	uri.m_port = m_port;
	uri.m_path = "/";
	return uri;
}

bool Uri::validate()
{
	return validate(toString());
}

bool Uri::validate(const utf8string &uri_string)
{
	details::inner_parse_out out;
	return out.parse_from(uri_string.c_str());
}

bool Uri::isEmpty() const
{
	return m_host.empty() || m_host == "/";
}

bool Uri::operator==(const Uri &other) const
{
	// Each individual URI component must be decoded before performing comparison.
	// TFS # 375865
	if (isEmpty() && other.isEmpty())
		return true;
	else if ((isEmpty() || other.isEmpty()) ||
			 (m_scheme != other.m_scheme) || // scheme is canonicalized to lowercase
			 (UriCodec::decode(m_user_info) != UriCodec::decode(other.m_user_info)) ||
			 (UriCodec::decode(m_host) != UriCodec::decode(other.m_host)) || // host is canonicalized to lowercase
			 (m_port != other.m_port) ||
			 (UriCodec::decode(m_path) != UriCodec::decode(other.m_path)) ||
			 (UriCodec::decode(m_query) != UriCodec::decode(other.m_query)) ||
			 (UriCodec::decode(m_fragment) != UriCodec::decode(other.m_fragment))
			)
		return false;

	return true;
}

void Uri::append_query_encode_impl(const utf8string &name, const utf8string &value, bool shouldEncode)
{
	if (shouldEncode)
	{
		auto encodeCondition = [](int ch) -> bool {
			switch (ch)
			{
				// Encode '&', ';', and '=' since they are used
				// as delimiters in query component.
				case '&':
				case ';':
				case '=':
				case '%':
				case '+': return true;
				default: return !details::is_query_character(ch);
			}
		};
		utf8string encodedQuery = details::encode_impl(name, encodeCondition);
		encodedQuery.push_back('=');
		encodedQuery.append(details::encode_impl(value, encodeCondition));

		// The query key value pair was already encoded by us or the user separately.
		appendQuery(encodedQuery, false);
	}
	else
	{
		appendQuery(name + "=" + value, false);
	}
}

utf8string UriCodec::encode(const utf8string &raw, UriComponent component)
{
	// Note: we also encode the '+' character because some non-standard implementations
	// encode the space character as a '+' instead of %20. To better interoperate we encode
	// '+' to avoid any confusion and be mistaken as a space.
	switch (component)
	{
		case UriComponent::userInfo:
			return details::encode_impl(raw, [](int ch) -> bool {
											return !details::is_user_info_character(ch) || ch == '%' || ch == '+';
										}
			);
		case UriComponent::host:
			return details::encode_impl(raw, [](int ch) -> bool {
											// No encoding of ASCII characters in host name (RFC 3986 3.2.2)
											return ch > 127;
										}
			);
		case UriComponent::path:
			return details::encode_impl(
					raw, [](int ch) -> bool { return !details::is_path_character(ch) || ch == '%' || ch == '+'; }
			);
		case UriComponent::query:
			return details::encode_impl(
					raw, [](int ch) -> bool { return !details::is_query_character(ch) || ch == '%' || ch == '+'; }
			);
		case UriComponent::fragment:
			return details::encode_impl(
					raw, [](int ch) -> bool { return !details::is_fragment_character(ch) || ch == '%' || ch == '+'; }
			);
		case UriComponent::fullUri:
		default:
			return details::encode_impl(
					raw, [](int ch) -> bool { return !details::is_unreserved(ch) && !details::is_reserved(ch); }
			);
	};
}

utf8string UriCodec::decode(const utf8string &encoded)
{
	utf8string raw;
	for (auto iter = encoded.begin(); iter != encoded.end(); ++iter)
	{
		if (*iter == '%')
		{
			if (++iter == encoded.end())
			{
				throw UriException("Invalid URI string, two hexadecimal digits must follow '%'");
			}
			int decimal_value = details::hex_char_digit_to_decimal_char(static_cast<int>(*iter)) << 4;
			if (++iter == encoded.end())
			{
				throw UriException("Invalid URI string, two hexadecimal digits must follow '%'");
			}
			decimal_value += details::hex_char_digit_to_decimal_char(static_cast<int>(*iter));

			raw.push_back(static_cast<char>(decimal_value));
		}
//		else if (*iter > 127 || *iter < 0)
//		{
//			throw UriException("Invalid encoded URI string, must be entirely ascii");
//		}
		else
		{
			// encoded string has to be ASCII.
			raw.push_back(static_cast<char>(*iter));
		}
	}
	return raw;
}
}
