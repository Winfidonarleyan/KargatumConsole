/*

 */


#ifndef _WARHEAD_LOG_COMMON_H_
#define _WARHEAD_LOG_COMMON_H_

#include "Define.h"
#include <string_view>

namespace Warhead
{
    enum class LogLevel : int8
    {
        Disabled,
        Fatal,
        Critical,
        Error,
        Warning,
        Info,
        Debug,
        Trace,

        Max
    };

    constexpr std::string_view GetLogLevelName(LogLevel level)
    {
        switch (level)
        {
        case Warhead::LogLevel::Disabled:
            return "Disabled";
        case Warhead::LogLevel::Fatal:
            return "Fatal";
        case Warhead::LogLevel::Critical:
            return "Critical";
        case Warhead::LogLevel::Error:
            return "Error";
        case Warhead::LogLevel::Warning:
            return "Warning";
        case Warhead::LogLevel::Info:
            return "Info";
        case Warhead::LogLevel::Debug:
            return "Debug";
        case Warhead::LogLevel::Trace:
            return "Trace";
        case Warhead::LogLevel::Max:
            return "Max";
        default:
            return "Unknown";
        }
    }

    enum class ChannelType : int8
    {
        Console = 1,
        File,
        DB,
        Discord,

        Max
    };

    constexpr std::size_t MAX_LOG_LEVEL = static_cast<std::size_t>(LogLevel::Max);
    constexpr std::size_t MAX_CHANNEL_TYPE = static_cast<std::size_t>(ChannelType::Max);
    constexpr std::size_t MAX_CHANNEL_OPTIONS = 7;
    constexpr std::size_t LOGGER_OPTIONS = 2;

} // namespace Warhead

#endif //
