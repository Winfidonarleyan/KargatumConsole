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

#include "Timer.h"
#include "StringFormat.h"
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>
#include <sstream>

template<>
WH_COMMON_API uint32 Warhead::Time::TimeStringTo<Seconds>(std::string_view timestring)
{
    uint32 secs = 0;
    uint32 buffer = 0;
    uint32 multiplier = 0;

    for (char itr : timestring)
    {
        if (isdigit(itr))
        {
            buffer *= 10;
            buffer += itr - '0';
        }
        else
        {
            switch (itr)
            {
            case 'd':
                multiplier = DAY;
                break;
            case 'h':
                multiplier = HOUR;
                break;
            case 'm':
                multiplier = MINUTE;
                break;
            case 's':
                multiplier = 1;
                break;
            default:
                return 0; // bad format
            }

            buffer *= multiplier;
            secs += buffer;
            buffer = 0;
        }
    }

    return secs;
}

template<>
WH_COMMON_API std::string Warhead::Time::ToTimeString<Microseconds>(uint64 durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    Poco::Timespan span(durationTime);

    uint32 millisecs = span.milliseconds();
    uint32 secs = span.seconds();
    uint32 minutes = span.minutes();
    uint32 hours = span.hours();
    uint32 days = span.days();

    if (timeFormat == TimeFormat::Numeric)
    {
        if (days)
            return Warhead::StringFormat("%u:%02u:%02u:%02u:%02u", days, hours, minutes, secs, millisecs);
        else if (hours)
            return Warhead::StringFormat("%u:%02u:%02u:%02u", hours, minutes, secs, millisecs);
        else if (minutes)
            return Warhead::StringFormat("%u:%02u:%02u", minutes, secs, millisecs);
        else if (secs)
            return Warhead::StringFormat("%u:%02u", secs, millisecs);
        else // millisecs
            return Warhead::StringFormat("%u", millisecs);
    }

    std::ostringstream ss;
    std::string stringTime;

    auto GetStringFormat = [&](uint32 timeType, std::string_view shortText, std::string_view fullText1, std::string_view fullText)
    {
        ss << timeType;

        switch (timeFormat)
        {
            case TimeFormat::ShortText:
                ss << shortText;
                break;
            case TimeFormat::FullText:
                ss << (timeType == 1 ? fullText1 : fullText);
                break;
            default:
                ss << "<Unknown time format>";
        }
    };

    if (days || timeOutput == TimeOutput::Days)
        GetStringFormat(days, "d ", " Day ", " Days ");

    if (timeOutput == TimeOutput::Days)
        stringTime = ss.str();

    if (hours || timeOutput == TimeOutput::Hours)
        GetStringFormat(hours, "h ", " Hour ", " Hours ");

    if (timeOutput == TimeOutput::Hours)
        stringTime = ss.str();

    if (minutes || timeOutput == TimeOutput::Minutes)
        GetStringFormat(minutes, "m ", " Minute ", " Minutes ");

    if (timeOutput == TimeOutput::Minutes)
        stringTime = ss.str();

    if (secs || timeOutput == TimeOutput::Seconds)
        GetStringFormat(secs, "s ", " Second ", " Seconds ");

    if (timeOutput == TimeOutput::Seconds)
        stringTime = ss.str();

    if (millisecs || timeOutput == TimeOutput::Milliseconds)
        GetStringFormat(millisecs, "ms ", " Millisecond ", " Milliseconds ");

    if (timeOutput == TimeOutput::Milliseconds)
        stringTime = ss.str();

    return Warhead::String::TrimRightInPlace(stringTime);
}

template<>
WH_COMMON_API std::string Warhead::Time::ToTimeString<Milliseconds>(uint64 durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    return ToTimeString<Microseconds>(durationTime * Poco::Timespan::MILLISECONDS, timeOutput, timeFormat);
}

template<>
WH_COMMON_API std::string Warhead::Time::ToTimeString<Seconds>(uint64 durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    return ToTimeString<Microseconds>(durationTime * Poco::Timespan::SECONDS, timeOutput, timeFormat);
}

template<>
WH_COMMON_API std::string Warhead::Time::ToTimeString<Minutes>(uint64 durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    return ToTimeString<Microseconds>(durationTime * Poco::Timespan::MINUTES, timeOutput, timeFormat);
}

template<>
WH_COMMON_API std::string Warhead::Time::ToTimeString<Seconds>(std::string_view durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    return ToTimeString<Seconds>(TimeStringTo<Seconds>(durationTime), timeOutput, timeFormat);
}

std::string Warhead::Time::ToTimeString(Microseconds durationTime, TimeOutput timeOutput /*= TimeOutput::Seconds*/, TimeFormat timeFormat /*= TimeFormat::ShortText*/)
{
    return ToTimeString<Microseconds>(durationTime.count(), timeOutput, timeFormat);
}

std::string Warhead::Time::TimeToTimestampStr(time_t t)
{
    return Poco::DateTimeFormatter::format(Poco::Timestamp::fromEpochTime(t), Poco::DateTimeFormat::RFC1123_FORMAT);
}
