#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::logger;
std::shared_ptr<spdlog::logger> Logger::assertLogger;

void Logger::Init()
{
    spdlog::set_level(spdlog::level::trace);

    logger = spdlog::stdout_color_mt("SGL");

    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%^[%T]: %v%$");

    assertLogger = spdlog::stdout_color_mt("SGLA");
    assertLogger->set_level(spdlog::level::trace);
    assertLogger->set_pattern("%^[%T]: %s:%#:%!() %v%$");
}