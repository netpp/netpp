//
// Created by gaojian on 2020/7/11.
//

#include "support/Log.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"
#include <iostream>

namespace netpp {
std::shared_ptr<spdlog::logger> logger = nullptr;

void initLogger(std::string logfile)
{
	std::once_flag loggerInitFlag;
	std::call_once(loggerInitFlag, [&]{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	//	console_sink->set_level(spdlog::level::trace);
	//	console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");
		auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logfile, 0, 0);
	//	file_sink->set_level(spdlog::level::trace);
	//	std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt<spdlog::async_factory>("console");
	//	std::shared_ptr<spdlog::logger> file = spdlog::daily_logger_mt<spdlog::async_factory>("logger", logfile);

	//	log = spdlog::stdout_color_mt("console");
	//	log = std::make_shared<spdlog::async_logger>("multi_sink",
		spdlog::init_thread_pool(50, std::thread::hardware_concurrency() > 2 ? 2 : 1);
		logger = std::make_shared<spdlog::async_logger>("netpp_sink",
														std::initializer_list<std::shared_ptr<spdlog::sinks::sink>>{
																console_sink, file_sink},
														spdlog::thread_pool());
		logger->set_level(spdlog::level::level_enum::trace);
		logger->set_pattern("[%Y-%m-%d %T.%e] [pid %5P] [tid %5t] [%=8l] %@ %v");
		spdlog::set_error_handler(
				[](const std::string &msg) { std::cerr << "Log error handler: " << msg << std::endl; }
		);
	});
}
}
