//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_LOG_H
#define NETPP_LOG_H

#undef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#define LOG_TRACE(args...)		if (netpp::logger) SPDLOG_LOGGER_TRACE(netpp::logger, ##args)
#define LOG_DEBUG(args...)		if (netpp::logger) SPDLOG_LOGGER_DEBUG(netpp::logger, ##args)
#define LOG_INFO(args...)		if (netpp::logger) SPDLOG_LOGGER_INFO(netpp::logger, ##args)
#define LOG_WARN(args...)		if (netpp::logger) SPDLOG_LOGGER_WARN(netpp::logger, ##args)
#define LOG_ERROR(args...)		if (netpp::logger) SPDLOG_LOGGER_ERROR(netpp::logger, ##args)
#define LOG_CRITICAL(args...)	if (netpp::logger) SPDLOG_LOGGER_CRITICAL(netpp::logger, ##args)

#include <memory>
#include <string>
#include "spdlog/spdlog.h"

namespace spdlog {
class logger;
}

namespace netpp {
	/**
	 * @brief init netpp internal logger, will be called before EventLoop starts
	 * 
	 * @param logfile log to file
	 */
	extern void initLogger(std::string logfile = "netpp.log");

	extern std::shared_ptr<spdlog::logger> logger;
}

#endif //NETPP_LOG_H
