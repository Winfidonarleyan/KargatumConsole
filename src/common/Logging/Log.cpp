/*
 * This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Log.h"
#include "Poco/WindowsConsoleChannel.h"
#include "Util.h"
#include <Poco/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/SplitterChannel.h>
#include <filesystem>
#include <sstream>

using namespace Poco;

namespace
{
    LogLevel highestLogLevel;
}

Log::Log()
{
    Initialize();
}

Log::~Log()
{
    Clear();
}

Log* Log::instance()
{
    static Log instance;
    return &instance;
}

void Log::Clear()
{
    // Clear all loggers
    Logger::shutdown();
}

void Log::Initialize()
{
    highestLogLevel = LOG_LEVEL_FATAL;

    Clear();
    InitSystemLogger();
}

void Log::InitSystemLogger()
{
    LogLevel level = LOG_LEVEL_DEBUG;

    // Start console channel
    AutoPtr<PatternFormatter> _ConsolePattern(new PatternFormatter);

    try
    {
        _ConsolePattern->setProperty("pattern", "%H:%M:%S %t");
        _ConsolePattern->setProperty("times", "local");
    }
    catch (const Poco::Exception& e)
    {
        fmt::print("Log::InitSystemLogger - {}\n", e.displayText().c_str());
    }

    AutoPtr<WindowsColorConsoleChannel> _ConsoleChannel(new WindowsColorConsoleChannel);

    try
    {
        _ConsoleChannel->setProperty("fatalColor", "lightRed");
        _ConsoleChannel->setProperty("criticalColor", "lightRed");
        _ConsoleChannel->setProperty("errorColor", "red");
        _ConsoleChannel->setProperty("warningColor", "brown");
        _ConsoleChannel->setProperty("noticeColor", "magenta");
        _ConsoleChannel->setProperty("informationColor", "cyan");
        _ConsoleChannel->setProperty("debugColor", "lightMagenta");
        _ConsoleChannel->setProperty("traceColor", "green");
    }
    catch (const Poco::Exception& e)
    {
        fmt::print("Log::InitSystemLogger - {}\n", e.displayText().c_str());
    }

    try
    {
        Logger::create("system", new FormattingChannel(_ConsolePattern, _ConsoleChannel), level);
    }
    catch (const Poco::Exception& e)
    {
        fmt::print("Log::InitSystemLogger - {}\n", e.displayText().c_str());
    }

    highestLogLevel = level;
}

bool Log::ShouldLog(LogLevel const level) const
{
    // Don't even look for a logger if the LogLevel is higher than the highest log levels across all loggers
    if (level > highestLogLevel)
        return false;

    Logger& logger = Logger::get("system");

    LogLevel logLevel = LogLevel(logger.getLevel());
    return logLevel != LOG_LEVEL_DISABLED && logLevel >= level;
}

void Log::SetLogLevel(LogLevel const level)
{
    highestLogLevel = level;

    Logger::get("system").setLevel(level);
}

void Log::_outSys(LogLevel level, std::string_view message)
{
    Logger& logger = Logger::get("system");

    try
    {
        switch (level)
        {
        case LOG_LEVEL_FATAL:
            logger.fatal(message.data());
            break;
        case LOG_LEVEL_CRITICAL:
            logger.critical(message.data());
            break;
        case LOG_LEVEL_ERROR:
            logger.error(message.data());
            break;
        case LOG_LEVEL_WARNING:
            logger.warning(message.data());
            break;
        case LOG_LEVEL_NOTICE:
            logger.notice(message.data());
            break;
        case LOG_LEVEL_INFO:
            //logger.information(fmt::format(message));
            logger.information(std::string(message));
            break;
        case LOG_LEVEL_DEBUG:
            logger.debug(message.data());
            break;
        case LOG_LEVEL_TRACE:
            logger.trace(message.data());
            break;
        default:
            break;
        }
    }
    catch (const Poco::Exception& e)
    {
        fmt::print("Log::outSys - {}\n", e.displayText());
    }
}
