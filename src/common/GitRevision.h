/*
 * Copyright (C)
 */

#ifndef __GITREVISION_H__
#define __GITREVISION_H__

#include "Define.h"

namespace GitRevision
{
    K_COMMON_API char const* GetHash();
    K_COMMON_API char const* GetDate();
    K_COMMON_API char const* GetBranch();
    K_COMMON_API char const* GetCMakeCommand();
    K_COMMON_API char const* GetCMakeVersion();
    K_COMMON_API char const* GetHostOSVersion();
    K_COMMON_API char const* GetBuildDirectory();
    K_COMMON_API char const* GetSourceDirectory();
    K_COMMON_API char const* GetMySQLExecutable();
    K_COMMON_API char const* GetFullDatabase();
    K_COMMON_API char const* GetFullVersion();
    K_COMMON_API char const* GetCompanyNameStr();
    K_COMMON_API char const* GetLegalCopyrightStr();
    K_COMMON_API char const* GetFileVersionStr();
    K_COMMON_API char const* GetProductVersionStr();
}

#endif
