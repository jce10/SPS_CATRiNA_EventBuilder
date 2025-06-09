/* Logger.cpp
*
* Logger.cpp is a wrapper that defines a pattern for logging messages and prints to the console. It utilizes spdlog.
* The different log levels (critical, error, warn, info, trace) can be used to log messages (see Logger.h).
* The pattern is as follows:
*
* [%T] = time
* %n = logger name
* %v = message
* %$ = reset color
* %^ = start color
*
*/

#include "spdlog/sinks/stdout_color_sinks.h"

namespace EventBuilder {

	std::shared_ptr<spdlog::logger> Logger::s_logger;

	void Logger::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_logger = spdlog::stdout_color_mt("EVB");
		s_logger->set_level(spdlog::level::trace);
	}

}