/*
 * Copyright (C)
 */

#ifndef WH_DEFINE_H
#define WH_DEFINE_H

#include "CompilerDefs.h"

#include <cinttypes>
#include <climits>
#include <cstddef>

#if WH_PLATFORM == WH_PLATFORM_WINDOWS
#  define WH_PATH_MAX 260
#  define _USE_MATH_DEFINES
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#endif // WH_PLATFORM

#if WH_COMPILER == WH_COMPILER_GNU
#  define ATTR_NORETURN __attribute__((__noreturn__))
#  define ATTR_PRINTF(F, V) __attribute__ ((__format__ (__printf__, F, V)))
#  define ATTR_DEPRECATED __attribute__((__deprecated__))
#else //WH_COMPILER != WH_COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#  define ATTR_DEPRECATED
#endif //WH_COMPILER == WH_COMPILER_GNU

#ifdef WH_API_USE_DYNAMIC_LINKING
#  if WH_COMPILER == WH_COMPILER_MICROSOFT
#    define K_API_EXPORT __declspec(dllexport)
#    define K_API_IMPORT __declspec(dllimport)
#  else
#    error compiler not supported!
#  endif
#else
#  define K_API_EXPORT
#  define K_API_IMPORT
#endif

#ifdef WH_API_EXPORT_COMMON
#  define WH_COMMON_API K_API_EXPORT
#else
#  define WH_COMMON_API K_API_IMPORT
#endif

#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%" PRId64
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#endif //WH_DEFINE_H
