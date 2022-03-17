#ifndef NETPP_URL_H
#define NETPP_URL_H

#include <string>
#include <vector>
#include <map>
#include "Types.h"
#include "error/Exception.h"

namespace netpp::http {
class Uri {
public:
	Uri();
	explicit Uri(const char *uri);
	explicit Uri(const utf8string &uri);

	[[nodiscard]] utf8string scheme() const { return m_scheme; }
	Uri &setScheme(const utf8string &scheme) { m_scheme = scheme; return *this; }

	[[nodiscard]] utf8string host() const { return m_host; }
	Uri &setHost(const utf8string &host, bool shouldEncode = true);

	[[nodiscard]] utf8string userInfo() const { return m_user_info; }
	Uri &setUserInfo(const utf8string &userInfo, bool shouldEncode = true);

	[[nodiscard]] utf8string path() const { return m_path; }
	Uri &setPath(const utf8string &path, bool shouldEncode = true);
	Uri &appendPath(const utf8string &path, bool shouldEncode = true);

	[[nodiscard]] int port() const { return m_port; }
	Uri &setPort(int port) { m_port = port; return *this; }

	[[nodiscard]] utf8string query() const { return m_query; }
	Uri &setQuery(const utf8string &query, bool shouldEncode = true);
	Uri &appendQuery(const utf8string &query, bool shouldEncode = true);
	template<typename T>
	Uri &appendQuery(const utf8string &name, const T &value, bool shouldEncode = true)
	{
		append_query_encode_impl(name, toUtf8String(value), shouldEncode);
		return *this;
	}

	[[nodiscard]] utf8string fragment() const { return m_fragment; }
	Uri &setFragment(const utf8string &fragment, bool shouldEncode = true);

	[[nodiscard]] Uri resolveUri(const utf8string &relativeUri) const;

	/**
	 * @brief Encode the Uri with URI encode
	 * @return encoded uri
	 */
	[[nodiscard]] utf8string toString();

	bool validate();

	/**
	 * @brief Splits a path into its hierarchical components.
	 * @param path The path as a string
	 * @return containing the segments in the path.
	 */
	std::vector<utf8string> splitPath();

	/**
	 * @brief Splits a query into its key-value components.
	 * @param query The query string
	 * @return containing the key-value components of the query.
	 */
	std::map<utf8string, utf8string> splitQuery();

	[[nodiscard]] Uri authority() const;

	static bool validate(const utf8string &uri);

	/**
	 * @brief An empty URI specifies no components, and serves as a default value
	 */
	[[nodiscard]] bool isEmpty() const;

	bool operator==(const Uri &other) const;
private:
	void append_query_encode_impl(const utf8string& name, const utf8string& value, bool shouldEncode);

private:
	utf8string m_scheme;
	utf8string m_host;
	utf8string m_user_info;
	utf8string m_path;
	utf8string m_query;
	utf8string m_fragment;
	int m_port;
};

class UriCodec {
public:
	enum class UriComponent {
		userInfo,
		host,
		path,
		query,
		fragment,
		fullUri
	};

	static utf8string encode(const utf8string &raw, UriComponent component = UriComponent::fullUri);
	static utf8string decode(const utf8string &raw);
};

/**
 * @brief A single exception type to represent errors in parsing, encoding, and decoding URIs.
 *
 */
class UriException : public error::Exception {
public:
	explicit UriException(std::string msg) : m_msg(std::move(msg)) {}
	[[nodiscard]] const char* what() const noexcept override { return m_msg.c_str(); }

private:
	std::string m_msg;
};
}

#endif //NETPP_URL_H
