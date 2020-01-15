/*
 * Copyright (C) 2019+ WarheadCore
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
#include "Util.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/SplitterChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include <sstream>

#if KARGATUM_PLATFORM == KARGATUM_PLATFORM_WINDOWS
#include "Poco/WindowsConsoleChannel.h"
#include <filesystem>
#define CONSOLE_CHANNEL WindowsColorConsoleChannel
#else
#include "Poco/ConsoleChannel.h"
#define CONSOLE_CHANNEL ColorConsoleChannel
#endif

using namespace Poco;

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
    Clear();
    InitSystemLogger();
    InitLogsDir();
}

void Log::InitSystemLogger()
{
    // Start console channel
    AutoPtr<PatternFormatter> _ConsolePattern(new PatternFormatter);

#define LOG_CATCH \
        catch (const std::exception& e) \
        { \
            printf("Log::InitSystemLogger - %s\n", e.what()); \
        } \

    try
    {
        _ConsolePattern->setProperty("pattern", "%t");
        _ConsolePattern->setProperty("times", "local");
    }
    LOG_CATCH

    AutoPtr<CONSOLE_CHANNEL> _ConsoleChannel(new CONSOLE_CHANNEL);

    try
    {
        _ConsoleChannel->setProperty("informationColor", "cyan");
    }
    LOG_CATCH

    // Start file channel
    AutoPtr<FileChannel> _FileChannel(new FileChannel);

    try
    {
        _FileChannel->setProperty("path", "System.log");
        _FileChannel->setProperty("times", "local");
        _FileChannel->setProperty("flush", "false");
    }
    LOG_CATCH

    AutoPtr<PatternFormatter> _FilePattern(new PatternFormatter);

    try
    {
        _FilePattern->setProperty("pattern", "%Y-%m-%d %H:%M:%S %t");
        _FilePattern->setProperty("times", "local");
    }
    LOG_CATCH

    AutoPtr<SplitterChannel> _split(new SplitterChannel);

    try
    {
        _split->addChannel(new FormattingChannel(_ConsolePattern, _ConsoleChannel));
        _split->addChannel(new FormattingChannel(_FilePattern, _FileChannel));
    }
    LOG_CATCH

    try
    {
        Logger::create("system", _split, 6);
    }
    LOG_CATCH
}

void Log::InitLogsDir()
{
    m_logsDir = ".";

    if (!m_logsDir.empty())
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.push_back('/');

#if KARGATUM_PLATFORM == KARGATUM_PLATFORM_WINDOWS
    std::filesystem::path LogsPath(m_logsDir);
    if (!std::filesystem::is_directory(LogsPath))
        m_logsDir = "";
#endif
}

bool Log::ShouldLog(LogLevel const level) const
{
    return Logger::get("system").getLevel() >= level;
}

void Log::SetLogLevel(LogLevel const level)
{
    Logger::get("system").setLevel(level);
}

void Log::outSys(LogLevel const level, std::string&& message)
{
    Logger& logger = Logger::get("system");

    try
    {
        switch (level)
        {
        case LOG_LEVEL_FATAL:
            logger.fatal(message);
            break;
        case LOG_LEVEL_CRITICAL:
            logger.critical(message);
            break;
        case LOG_LEVEL_ERROR:
            logger.error(message);
            break;
        case LOG_LEVEL_WARNING:
            logger.warning(message);
            break;
        case LOG_LEVEL_NOTICE:
            logger.notice(message);
            break;
        case LOG_LEVEL_INFO:
            logger.information(message);
            break;
        case LOG_LEVEL_DEBUG:
            logger.debug(message);
            break;
        case LOG_LEVEL_TRACE:
            logger.trace(message);
            break;
        default:
            break;
        }
    }
    catch (const std::exception& e)
    {
        printf("Log::outSys - %s\n", e.what());
    }
}
