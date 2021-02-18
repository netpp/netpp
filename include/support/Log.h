//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_LOG_H
#define NETPP_LOG_H

#undef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#define LOG_TRACE(args...)		if (netpp::internal::logger) SPDLOG_LOGGER_TRACE(netpp::internal::logger, ##args)
#define LOG_DEBUG(args...)		if (netpp::internal::logger) SPDLOG_LOGGER_DEBUG(netpp::internal::logger, ##args)
#define LOG_INFO(args...)		if (netpp::internal::logger) SPDLOG_LOGGER_INFO(netpp::internal::logger, ##args)
#define LOG_WARN(args...)		if (netpp::internal::logger) SPDLOG_LOGGER_WARN(netpp::internal::logger, ##args)
#define LOG_ERROR(args...)		if (netpp::internal::logger) SPDLOG_LOGGER_ERROR(netpp::internal::logger, ##args)
#define LOG_CRITICAL(args...)	if (netpp::internal::logger) SPDLOG_LOGGER_CRITICAL(netpp::internal::logger, ##args)

#include <memory>
#include <string>
#include "spdlog/spdlog.h"

namespace spdlog {
class logger;
}

namespace netpp::internal {
	/**
	 * @brief init netpp internal logger, will be called before EventLoop starts
	 * 
	 * @param logfile log to file
	 */
	extern void initLogger(std::string logfile = "netpp.log");

	extern std::shared_ptr<spdlog::logger> logger;
}

#endif //NETPP_LOG_H
