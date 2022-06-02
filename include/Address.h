//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_ADDRESS_H
#define NETPP_ADDRESS_H

#include <string>
#include <memory>

struct sockaddr_in;

namespace netpp {
/**
 * @brief The ipv4 address
 * @todo support ipv6
 */
class Address {
public:
	/**
	 * @brief Create
	 * @param ip	The ip address
	 * @param port	The port
	 */
	explicit Address(const std::string &ip = "0.0.0.0", unsigned port = 11111);

	/**
	 * @brief Get ip address
	 * @return ip string
	 */
	[[nodiscard]] std::string ip() const;
	/**
	 * @brief Get port
	 * @return port
	 */
	[[nodiscard]] unsigned port() const;

	// bool ipv6();

	/**
	 * @brief Get linux address
	 * @return linux address info
	 */
	inline ::sockaddr_in *sockAddrIn() { return m_addr.get(); }

private:
	std::shared_ptr<::sockaddr_in> m_addr;
};
}

#endif //NETPP_ADDRESS_H
