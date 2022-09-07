//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_ADDRESS_H
#define NETPP_ADDRESS_H

#include <string>

namespace netpp {
/**
 * @brief The ip address
 * @todo support ipv6
 */
class Address {
public:
	Address();
	/**
	 * @brief Create
	 * @param ip	The ip address
	 * @param port	The port
	 */
	Address(std::string ip, uint16_t port);

	bool isValid() const;

	/**
	 * @brief Get ip address
	 * @return ip string
	 */
	[[nodiscard]] std::string ip() const { return m_ipAddress; }
	/**
	 * @brief Get port
	 * @return port
	 */
	[[nodiscard]] uint16_t port() const { return m_port; }

	// bool ipv6();

private:
	std::string m_ipAddress;
	uint16_t m_port;
};
}

#endif //NETPP_ADDRESS_H
