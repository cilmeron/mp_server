#include "log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
#ifdef __linux
#elif _WIN32
	#include <Windows.h>
#else
#endif
namespace mp_server
{ 
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

	void Log::Init()
	{
//		SetConsoleOutputCP(CP_UTF8);
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Server");
		s_CoreLogger->set_level(spdlog::level::trace);
	}
}
