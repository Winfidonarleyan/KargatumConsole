/*
 * Copyright (C) 
 */

#ifndef KARGATUM_DEFINE_H
#define KARGATUM_DEFINE_H

#include "CompilerDefs.h"

#include <cstddef>
#include <cinttypes>
#include <climits>

#if KARGATUM_PLATFORM == KARGATUM_PLATFORM_WINDOWS
#  define KARGATUM_PATH_MAX 260
#  define _USE_MATH_DEFINES
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#endif // KARGATUM_PLATFORM


#if KARGATUM_COMPILER == KARGATUM_COMPILER_GNU
#  define ATTR_NORETURN __attribute__((__noreturn__))
#  define ATTR_PRINTF(F, V) __attribute__ ((__format__ (__printf__, F, V)))
#  define ATTR_DEPRECATED __attribute__((__deprecated__))
#else //KARGATUM_COMPILER != KARGATUM_COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#  define ATTR_DEPRECATED
#endif //KARGATUM_COMPILER == KARGATUM_COMPILER_GNU

#ifdef KARGATUM_API_USE_DYNAMIC_LINKING
#  if KARGATUM_COMPILER == KARGATUM_COMPILER_MICROSOFT
#    define K_API_EXPORT __declspec(dllexport)
#    define K_API_IMPORT __declspec(dllimport)
#  else
#    error compiler not supported!
#  endif
#else
#  define K_API_EXPORT
#  define K_API_IMPORT
#endif

#ifdef KARGATUM_API_EXPORT_COMMON
#  define K_COMMON_API K_API_EXPORT
#else
#  define K_COMMON_API K_API_IMPORT
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

#endif //KARGATUM_DEFINE_H
