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

#ifndef WARHEAD_TIMER_H
#define WARHEAD_TIMER_H

#include "Common.h"
#include "Duration.h"
#include <string_view>

enum class TimeFormat : uint8
{
    FullText,       // 1 Days 2 Hours 3 Minutes 4 Seconds 5 Milliseconds
    ShortText,      // 1d 2h 3m 4s 5ms
    Numeric         // 1:2:3:4:5
};

enum class TimeOutput : uint8
{
    Days,           // 1d
    Hours,          // 1d 2h
    Minutes,        // 1d 2h 3m
    Seconds,        // 1d 2h 3m 4s
    Milliseconds,   // 1d 2h 3m 4s 5ms
    Microseconds    // 1d 2h 3m 4s 5ms 6us
};

namespace Warhead::Time
{
    template<class T>
    WH_COMMON_API uint32 TimeStringTo(std::string_view timestring);

    template<class T>
    WH_COMMON_API std::string ToTimeString(uint64 durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);

    template<class T>
    WH_COMMON_API std::string ToTimeString(std::string_view durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);

    WH_COMMON_API std::string ToTimeString(Microseconds durationTime, TimeOutput timeOutput = TimeOutput::Seconds, TimeFormat timeFormat = TimeFormat::ShortText);

    WH_COMMON_API time_t LocalTimeToUTCTime(time_t time);
    WH_COMMON_API time_t GetLocalHourTimestamp(time_t time, uint8 hour, bool onlyAfterTime = true);
    WH_COMMON_API tm TimeBreakdown(time_t t);
    WH_COMMON_API std::string TimeToTimestampStr(time_t t);
    WH_COMMON_API std::string TimeToHumanReadable(time_t t);
}

inline TimePoint GetApplicationStartTime()
{
    static const TimePoint ApplicationStartTime = std::chrono::steady_clock::now();

    return ApplicationStartTime;
}

inline uint32 getMSTime()
{
    using namespace std::chrono;

    return uint32(duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime()).count());
}

inline uint32 getMSTimeDiff(uint32 oldMSTime, uint32 newMSTime)
{
    // getMSTime() have limited data range and this is case when it overflow in this tick
    if (oldMSTime > newMSTime)
        return (0xFFFFFFFF - oldMSTime) + newMSTime;
    else
        return newMSTime - oldMSTime;
}

inline uint32 getMSTimeDiff(uint32 oldMSTime, TimePoint newTime)
{
    using namespace std::chrono;

    uint32 newMSTime = uint32(duration_cast<milliseconds>(newTime - GetApplicationStartTime()).count());
    return getMSTimeDiff(oldMSTime, newMSTime);
}

inline uint32 GetMSTimeDiffToNow(uint32 oldMSTime)
{
    return getMSTimeDiff(oldMSTime, getMSTime());
}

#endif
