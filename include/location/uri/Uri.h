#ifndef NETPP_URL_H
#define NETPP_URL_H

#include <string>
#include <vector>
#include <map>
#include "support/Types.h"
#include "support/Util.h"

namespace netpp {
/**
 * @brief The Uniform Resource Identifier.
 * @section Uri components
 * @htmlonly
 * <pre>
 *         +-------------hierarchical part-----------+
 *         +------------authority----------+ +-path--+
 *         |                               | |       |
 *   abc://username:password@example.com:123/path/data?key=value&key2=value2#fragid1
 *   | |   |               | |         | | |           |                   | |     |
 *   +-+   +---user info---+ +--host---+ +-+           +------query--------+ +-----+
 * scheme                                port                                fragment
 * </pre>
 * @endhtmlonly
 */
class Uri {
public:
	Uri();
	/**
	 * @brief Parse uri string and construct an Uri object.
	 * @param uri The raw uri string
	 */
	explicit Uri(const char *uri);
	explicit Uri(const utf8string &uri);

	/**
	 * @brief Get scheme of this uri
	 * @return scheme string
	 */
	[[nodiscard]] utf8string scheme() const { return m_scheme; }
	/**
	 * @brief Set scheme of this uri
	 * @param scheme scheme string
	 * @return this Uri object, for chain calls
	 */
	Uri &setScheme(const utf8string &scheme) { m_scheme = scheme; return *this; }

	/**
	 * @brief Get the host of this uri
	 * @return host string
	 */
	[[nodiscard]] utf8string host() const { return m_host; }
	/**
	 * @brief Set host of this uri
	 * @param host			host string
	 * @param shouldEncode	should do uri encoding on host, by default true
	 * @return this Uri object, for chain calls
	 */
	Uri &setHost(const utf8string &host, bool shouldEncode = true);

	/**
	 * @brief Get user info of this uri
	 * @return user info string
	 */
	[[nodiscard]] utf8string userInfo() const { return m_user_info; }
	/**
	 * @brief Set user info of this uri
	 * @param userInfo		user info string
	 * @param shouldEncode	should do uri encoding on host, by default true
	 * @return this Uri object, for chain calls
	 */
	Uri &setUserInfo(const utf8string &userInfo, bool shouldEncode = true);

	/**
	 * @brief Get user info of this uri
	 * @return user info string
	 */
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

	/**
	 * @brief Splits a path into its hierarchical components.
	 * @param path The path as a string
	 * @return containing the segments in the path.
	 */
	std::vector<utf8string> splitPath();

	/**
	 * @brief Splits a query into its key-value components.
	 * @return containing the key-value components of the query.
	 */
	std::map<utf8string, utf8string> splitQuery();

	/**
	 * @brief Get authority part of this uri
	 * @return Uri contains authority party only
	 */
	[[nodiscard]] Uri authority() const;

	/**
	 * @brief Test if this uri is valid
	 * @return true if valid, otherwise false
	 */
	bool validate();
	/**
	 * @brief Test if a string is valid uri
	 * @param uri The uri string
	 * @return true if valid, otherwise false
	 */
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
}

#endif //NETPP_URL_H
