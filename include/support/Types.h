//
// Created by 17271 on 2022/6/9.
//

#ifndef NETPP_TYPES_H
#define NETPP_TYPES_H

#include <string>
#include <sstream>
#include <functional>
#include "error/Error.h"
#include "signal/Signals.h"
#include <memory>

namespace netpp {
using BufferLength = std::size_t;

class Channel;

/**
 * @brief The time interval type
 */
using TimerInterval = long int;

/**
 * @brief a simplify tcp state
 *
 */
enum class TcpState {
	Closed,			// at init state
	Listen,			// is listening
	Connecting,		// is connecting
	Established,	// connection established
	HalfClose		// is closing connection
};

/**
 * @brief The string support utf8 encoding
 */
using utf8string = std::string;

/** @brief Callback type when new connecting accepted */
using ConnectedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
/** @brief Callback type when message received */
using MessageReceivedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
/** @brief Callback type when write finished */
using WriteCompletedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
/** @brief Callback type when disconnected */
using DisconnectedCallBack = std::function<void(std::shared_ptr<netpp::Channel>)>;
/** @brief Callback type when some error occurred */
using ErrorCallBack = std::function<void(Error)>;
/** @brief Callback type when signal emits */
using SignalCallBack = std::function<void(Signals)>;
}

#endif //NETPP_TYPES_H
