/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WARHEAD_TIMER_H
#define WARHEAD_TIMER_H

#include "Define.h"
#include "Duration.h"

enum class TimeFormat : uint8
{
    FullText,       // 1 Days 2 Hours 3 Minutes 4 Seconds 5 Milliseconds 6 Microseconds
    ShortText,      // 1d 2h 3m 4s 5ms 6us
    Numeric         // 1:2:3:4:5:6
};

namespace Warhead::Time
{
    WH_COMMON_API std::string ToTimeString(Microseconds durationTime, uint8 outCount = 3, TimeFormat timeFormat = TimeFormat::ShortText);

    WH_COMMON_API time_t LocalTimeToUTCTime(time_t time);
    WH_COMMON_API time_t GetLocalHourTimestamp(time_t time, uint8 hour, bool onlyAfterTime = true);
    WH_COMMON_API std::tm TimeBreakdown(time_t t = 0);
    WH_COMMON_API std::string TimeToTimestampStr(Seconds time = 0s, std::string_view fmt = {});
    WH_COMMON_API std::string TimeToHumanReadable(Seconds time = 0s, std::string_view fmt = {});
}

#if WARHEAD_PLATFORM == WARHEAD_PLATFORM_WINDOWS
WH_COMMON_API struct tm* localtime_r(time_t const* time, struct tm* result);
#endif

inline TimePoint GetApplicationStartTime()
{
    static const TimePoint ApplicationStartTime = std::chrono::steady_clock::now();

    return ApplicationStartTime;
}

inline Milliseconds GetTimeMS()
{
    using namespace std::chrono;

    return duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime());
}

inline Milliseconds GetMSTimeDiff(Milliseconds oldMSTime, Milliseconds newMSTime)
{
    if (oldMSTime > newMSTime)
    {
        return oldMSTime - newMSTime;
    }
    else
    {
        return newMSTime - oldMSTime;
    }
}

inline Milliseconds GetMSTimeDiffToNow(Milliseconds oldMSTime)
{
    return GetMSTimeDiff(oldMSTime, GetTimeMS());
}

inline Seconds GetEpochTime()
{
    using namespace std::chrono;
    return duration_cast<Seconds>(system_clock::now().time_since_epoch());
}

#endif
