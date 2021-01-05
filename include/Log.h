//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_LOG_H
#define NETPP_LOG_H

#undef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <memory>
#include <string>
#include "spdlog/spdlog.h"

namespace spdlog {
class logger;
}

namespace netpp {
	// WARNING: not thread safe
	extern void initLogger(std::string logfile = "netpp.log", spdlog::level::level_enum level = spdlog::level::trace);
	extern std::shared_ptr<spdlog::logger> logger;
}

#endif //NETPP_LOG_H
