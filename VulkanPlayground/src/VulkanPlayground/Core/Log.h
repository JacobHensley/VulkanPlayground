#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace VKPlayground {

	class Log
	{
	public:
		static void Init();

	public:
		inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

// Core log macros
#define LOG_TRACE(...)    VKPlayground::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    VKPlayground::Log::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     VKPlayground::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     VKPlayground::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    VKPlayground::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) VKPlayground::Log::GetLogger()->critical(__VA_ARGS__)