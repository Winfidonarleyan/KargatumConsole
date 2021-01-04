/*
 * Copyright (C)
 */

#ifndef _COMMON_H
#define _COMMON_H

#include "Define.h"
#include <memory>
#include <string>
#include <utility>

#if WH_PLATFORM == WH_PLATFORM_WINDOWS
#include <ws2tcpip.h>
#endif

#if WH_COMPILER == WH_COMPILER_MICROSOFT
#define atoll _atoi64
#define llabs _abs64
#endif

inline unsigned long atoul(char const* str) { return strtoul(str, nullptr, 10); }
inline unsigned long long atoull(char const* str) { return strtoull(str, nullptr, 10); }

#define STRINGIZE(a) #a

enum TimeConstants
{
    MINUTE          = 60,
    HOUR            = MINUTE*60,
    DAY             = HOUR*24,
    WEEK            = DAY*7,
    MONTH           = DAY*30,
    YEAR            = MONTH*12,
    IN_MILLISECONDS = 1000
};

// we always use stdlib std::max/std::min, undefine some not C++ standard defines (Win API and some other platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

#endif
