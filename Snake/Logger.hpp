#ifndef MOUNTAIN_LOGGER_HPP
#define MOUNTAIN_LOGGER_HPP

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Mountain 
{
	class Logger
	{

		public:
			static void Init();

			static std::shared_ptr<spdlog::logger> GetCoreLogger() { return CoreLogger; }
			static std::shared_ptr<spdlog::logger> GetClientLogger() { return ClientLogger; }


		private:
			Logger() = delete;
			Logger(const Logger&) = delete;
			Logger operator=(const Logger&) = delete;

			static std::shared_ptr<spdlog::logger> CoreLogger;
			static std::shared_ptr<spdlog::logger> ClientLogger;
	};
}

// Core log macros
#define MTN_CORE_TRACE(...)    ::Mountain::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define MTN_CORE_INFO(...)     ::Mountain::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define MTN_CORE_WARN(...)     ::Mountain::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define MTN_CORE_ERROR(...)    ::Mountain::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define MTN_CORE_CRITICAL(...) ::Mountain::Logger::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define MTN_TRACE(...)         ::Mountain::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define MTN_INFO(...)          ::Mountain::Logger::GetClientLogger()->info(__VA_ARGS__)
#define MTN_WARN(...)          ::Mountain::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define MTN_ERROR(...)         ::Mountain::Logger::GetClientLogger()->error(__VA_ARGS__)
#define MTN_CRITICAL(...)      ::Mountain::Logger::GetClientLogger()->critical(__VA_ARGS__)

#endif	// MOUNTAIN_LOGGER_HPP