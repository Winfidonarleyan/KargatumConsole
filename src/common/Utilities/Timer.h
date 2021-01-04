/*
 * Copyright (C)
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "Define.h"
#include <chrono>

inline std::chrono::steady_clock::time_point GetApplicationStartTime()
{
    using namespace std::chrono;

    static const steady_clock::time_point ApplicationStartTime = steady_clock::now();

    return ApplicationStartTime;
}

inline uint32 getMSTime()
{
    using namespace std::chrono;

    return uint32(duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime()).count());
}

inline uint32 getMKSTime()
{
    using namespace std::chrono;

    return uint32(duration_cast<microseconds>(steady_clock::now() - GetApplicationStartTime()).count());
}

inline uint32 getTimeDiff(uint32 oldMSTime, uint32 newMSTime)
{
    // getMSTime() have limited data range and this is case when it overflow in this tick
    if (oldMSTime > newMSTime)
        return (0xFFFFFFFF - oldMSTime) + newMSTime;
    else
        return newMSTime - oldMSTime;
}

inline uint32 getMSTimeDiff(uint32 oldMSTime, std::chrono::steady_clock::time_point newTime)
{
    using namespace std::chrono;

    uint32 newMSTime = uint32(duration_cast<milliseconds>(newTime - GetApplicationStartTime()).count());
    return getTimeDiff(oldMSTime, newMSTime);
}

inline uint32 getMKSTimeDiff(uint32 oldMSTime, std::chrono::steady_clock::time_point newTime)
{
    using namespace std::chrono;

    uint32 newMSTime = uint32(duration_cast<microseconds>(newTime - GetApplicationStartTime()).count());
    return getTimeDiff(oldMSTime, newMSTime);
}

inline uint32 GetMSTimeDiffToNow(uint32 oldMSTime)
{
    return getTimeDiff(oldMSTime, getMSTime());
}

inline uint32 GetMKSTimeDiffToNow(uint32 oldMSTime)
{
    return getTimeDiff(oldMSTime, getMKSTime());
}

#endif
