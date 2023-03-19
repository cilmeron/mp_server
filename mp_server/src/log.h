#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace mp_server
{
	class Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
		{
			return s_CoreLogger;
		}
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};
}

#define _CORE_TRACE(...) mp_server::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define _CORE_INFO(...) mp_server::Log::GetCoreLogger()->info(__VA_ARGS__)
#define _CORE_WARN(...) mp_server::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define _CORE_ERROR(...) mp_server::Log::GetCoreLogger()->error(__VA_ARGS__)
#define _CORE_FATAL(...) mp_server::Log::GetCoreLogger()->fatal(__VA_ARGS__)
