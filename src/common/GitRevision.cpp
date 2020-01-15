/*
 * Copyright (C) 
 */

#include "GitRevision.h"
#include "revision_data.h"

char const* GitRevision::GetHash()
{
    return _HASH;
}

char const* GitRevision::GetDate()
{
    return _DATE;
}

char const* GitRevision::GetBranch()
{
    return _BRANCH;
}

char const* GitRevision::GetCMakeCommand()
{
    return _CMAKE_COMMAND;
}

char const* GitRevision::GetCMakeVersion()
{
    return _CMAKE_VERSION;
}

char const* GitRevision::GetHostOSVersion()
{
    return _CMAKE_HOST_SYSTEM;
}

char const* GitRevision::GetBuildDirectory()
{
    return _BUILD_DIRECTORY;
}

char const* GitRevision::GetSourceDirectory()
{
    return _SOURCE_DIRECTORY;
}

char const* GitRevision::GetMySQLExecutable()
{
    return _MYSQL_EXECUTABLE;
}

#if KARGATUM_PLATFORM == KARGATUM_PLATFORM_WINDOWS
#  ifdef _WIN64
#    define KARGATUM_PLATFORM_STR "Win64"
#  else
#    define KARGATUM_PLATFORM_STR "Win32"
#  endif
#elif KARGATUM_PLATFORM == KARGATUM_PLATFORM_APPLE
#  define KARGATUM_PLATFORM_STR "MacOSX"
#elif KARGATUM_PLATFORM == KARGATUM_PLATFORM_INTEL
#  define KARGATUM_PLATFORM_STR "Intel"
#else // KARGATUM_PLATFORM_UNIX
#  define KARGATUM_PLATFORM_STR "Unix"
#endif

#ifndef KARGATUM_API_USE_DYNAMIC_LINKING
#  define KARGATUM_LINKAGE_TYPE_STR "Static"
#else
#  define KARGATUM_LINKAGE_TYPE_STR "Dynamic"
#endif

char const* GitRevision::GetFullVersion()
{
  return "KargatumConsole rev. " VER_PRODUCTVERSION_STR
    " (" KARGATUM_PLATFORM_STR ", " _BUILD_DIRECTIVE ", " KARGATUM_LINKAGE_TYPE_STR ")";
}

char const* GitRevision::GetCompanyNameStr()
{
    return VER_COMPANYNAME_STR;
}

char const* GitRevision::GetLegalCopyrightStr()
{
    return VER_LEGALCOPYRIGHT_STR;
}

char const* GitRevision::GetFileVersionStr()
{
    return VER_FILEVERSION_STR;
}

char const* GitRevision::GetProductVersionStr()
{
    return VER_PRODUCTVERSION_STR;
}
