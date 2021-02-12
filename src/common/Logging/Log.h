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

#ifndef _LOG_H
#define _LOG_H

#include "Common.h"
#include "StringFormat.h"

enum LogLevel
{
    LOG_LEVEL_DISABLED,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_CRITICAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,

    LOG_LEVEL_MAX
};

class WH_COMMON_API Log
{
private:
    Log();
    ~Log();
    Log(Log const&) = delete;
    Log(Log&&) = delete;
    Log& operator=(Log const&) = delete;
    Log& operator=(Log&&) = delete;

public:
    static Log* instance();

    void SetLogLevel(LogLevel const level);

    bool ShouldLog(LogLevel const level) const;

    template<typename Format, typename... Args>
    inline void outSys(LogLevel const level, Format&& fmt, Args&& ... args)
    {
        outSys(level, Warhead::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
    }

private:
    void outSys(LogLevel level, std::string&& message);

    void Initialize();
    void InitSystemLogger();
    void Clear();
};

#define sLog Log::instance()

#define LOG_EXCEPTION_FREE(level__, ...) \
    { \
        try \
        { \
            sLog->outSys(level__, __VA_ARGS__); \
        } \
        catch (const std::exception& e) \
        { \
            sLog->outSys(LOG_LEVEL_ERROR, "Wrong format occurred (%s) at %s:%u.", \
                e.what(), __FILE__, __LINE__); \
        } \
    }

#define LOG_MSG_BODY(level__, ...)                                      \
        do {                                                            \
            if (sLog->ShouldLog(level__))                               \
                LOG_EXCEPTION_FREE(level__, __VA_ARGS__);               \
        } while (0)

// Fatal - 1
#define LOG_FATAL(...) \
    LOG_MSG_BODY(LOG_LEVEL_FATAL, __VA_ARGS__)

// Critical - 2
#define LOG_CRIT(...) \
    LOG_MSG_BODY(LOG_LEVEL_CRITICAL, __VA_ARGS__)

// Error - 3
#define LOG_ERROR(...) \
    LOG_MSG_BODY(LOG_LEVEL_ERROR, __VA_ARGS__)

// Warning - 4
#define LOG_WARN(...)  \
    LOG_MSG_BODY(LOG_LEVEL_WARNING, __VA_ARGS__)

// Notice - 5
#define LOG_NOTICE(...)  \
    LOG_MSG_BODY(LOG_LEVEL_NOTICE, __VA_ARGS__)

// Info - 6
#define LOG_INFO(...)  \
    LOG_MSG_BODY(LOG_LEVEL_INFO, __VA_ARGS__)

// Debug - 7
#define LOG_DEBUG(...) \
    LOG_MSG_BODY(LOG_LEVEL_DEBUG, __VA_ARGS__)

// Trace - 8
#define LOG_TRACE(...) \
    LOG_MSG_BODY(LOG_LEVEL_TRACE, __VA_ARGS__)

#endif
