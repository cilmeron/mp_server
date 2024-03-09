#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

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

    class Logger 
    {
    public:
        Logger(const std::string& filename) : m_filename(filename) {}

        template<typename... Args>
        void writeLog(const std::string& format, Args... args)
        {
            std::ofstream logfile;
            logfile.open(m_filename, std::ios_base::app); // Append to the file

            if (!logfile.is_open())
            {
                std::cerr << "Error opening the log file: " << m_filename << std::endl;
                return;
            }

            time_t now = time(0);
            tm* localTime = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);
            std::ostringstream oss;
            print(oss, format, args...);
            logfile << "[" << timestamp << "] " << oss.str() << std::endl;
            logfile.close();
        }

    private:
        std::string m_filename;

        // Recursive variadic template function to format the log message
        template<typename T>
        void print(std::ostringstream& oss, const T& value)
        {
            oss << value;
        }

        template<typename T, typename... Args>
        void print(std::ostringstream& oss, const T& value, const Args&... args)
        {
            std::ostringstream tempStream;
            tempStream << value;
            std::string format = tempStream.str();
            size_t pos = 0;
            size_t openBrace = format.find("{");
            size_t closeBrace = format.find("}");

            if (openBrace != std::string::npos && closeBrace != std::string::npos && closeBrace > openBrace)
            {
                // Append the part before the open brace
                oss << format.substr(0, openBrace);

                // Get the index within the braces
                size_t index = std::stoul(format.substr(openBrace + 1, closeBrace - openBrace - 1));

                // Check if the index is valid
                if (index < sizeof...(args))
                {
                    expand_and_print(oss, index, args...);
                }

                // Recur with the rest of the string
                print(oss, format.substr(closeBrace + 1), args...);
            }
            else
            {
                // No more placeholders, just append the remaining format
                oss << format;
            }
        }

        template<typename T>
        void expand_and_print(std::ostringstream& oss, size_t index, const T& value)
        {
            if (index == 0)
            {
                oss << value;
            }
        }

        template<typename T, typename... Args>
        void expand_and_print(std::ostringstream& oss, size_t index, const T& value, const Args&... args)
        {
            if (index == 0)
            {
                oss << value;
            }
            else
            {
                expand_and_print(oss, index - 1, args...);
            }
        }


    };
}

#define _CORE_TRACE(...)\
do {\
 ::mp_server::Log::GetCoreLogger()->trace(__VA_ARGS__); \
 ::mp_server::Logger("mp_server_test.log").writeLog(__VA_ARGS__); \
} while(0)

#define _CORE_INFO(...)\
do {\
 ::mp_server::Log::GetCoreLogger()->info(__VA_ARGS__); \
 ::mp_server::Logger("mp_server_test.log").writeLog(__VA_ARGS__); \
} while(0)

#define _CORE_WARN(...)\
do {\
 ::mp_server::Log::GetCoreLogger()->warn(__VA_ARGS__); \
 ::mp_server::Logger("mp_server_test.log").writeLog(__VA_ARGS__); \
} while (0)

#define _CORE_ERROR(...)\
do {\
 ::mp_server::Log::GetCoreLogger()->error(__VA_ARGS__);\
 ::mp_server::Logger("mp_server_test.log").writeLog(__VA_ARGS__); \
} while (0)

#define _CORE_FATAL(...)\
do {\
 ::mp_server::Log::GetCoreLogger()->fatal(__VA_ARGS__); \
 ::mp_server::Logger("mp_server_test.log").writeLog(__VA_ARGS__); \
} while(0)
