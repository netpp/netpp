//
// Created by gaojian on 2022/3/29.
//

#ifndef NETPP_URICODEC_H
#define NETPP_URICODEC_H

#include "support/Types.h"

namespace netpp {
/**
 * @brief encode/decode uri components
 */
class UriCodec {
public:
	/**
	 * @brief The uri components
	 * @see Uri
	 */
	enum class UriComponent {
		/** uri user info part */
		userInfo,
		/** uri host part */
		host,
		/** uri path part */
		path,
		/** uri query part */
		query,
		/** uri fragment part */
		fragment,
		/** the whole uri */
		fullUri
	};

	/**
	 * @brief Encode with uri encoding
	 * @param raw		The string to be encoded
	 * @param component	What party the string is, by default UriComponent::fullUri
	 * @return Encoded string
	 */
	static utf8string encode(const utf8string &raw, UriComponent component = UriComponent::fullUri);

	/**
	 * @brief Decode string with uri encoding
	 * @param raw		The string to be encoded
	 * @return Decoded string
	 */
	static utf8string decode(const utf8string &raw);
};
}

#endif //NETPP_URICODEC_H
