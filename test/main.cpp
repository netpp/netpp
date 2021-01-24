#include <gtest/gtest.h>
#include "Log.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void initTestLogger()
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_st>("netpptest.log", 0, 0);
	netpp::logger = std::make_shared<spdlog::logger>("netpp_test_sink",
													std::initializer_list<std::shared_ptr<spdlog::sinks::sink>>{
															console_sink, file_sink});
	netpp::logger->set_level(spdlog::level::err);
	netpp::logger->flush_on(spdlog::level::err);
	netpp::logger->set_pattern("[%Y-%m-%d %T.%e] [pid %5P] [tid %5t] [%=8l] %@ %v");
	spdlog::set_error_handler(
			[](const std::string &msg) { std::cerr << "Log error handler: " << msg << std::endl; }
	);
}

int main(int argc, char *argv[])
{
	initTestLogger();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
