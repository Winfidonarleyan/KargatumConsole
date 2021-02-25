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

#ifndef _UTIL_H
#define _UTIL_H

#include "Define.h"
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Warhead
{
    WH_COMMON_API std::vector<std::string_view> Tokenize(std::string_view str, char sep, bool keepEmpty);

    /* this would return string_view into temporary otherwise */
    std::vector<std::string_view> Tokenize(std::string&&, char, bool) = delete;
    std::vector<std::string_view> Tokenize(std::string const&&, char, bool) = delete;

    /* the delete overload means we need to make this explicit */
    inline std::vector<std::string_view> Tokenize(char const* str, char sep, bool keepEmpty) { return Tokenize(std::string_view(str ? str : ""), sep, keepEmpty); }
}

WH_COMMON_API bool StringEqualI(std::string_view str1, std::string_view str2);

namespace Warhead::File
{
    WH_COMMON_API std::string GetFileText(std::string const& path, bool openBinary = false);
    WH_COMMON_API bool FindFile(std::string_view findName, std::string const& pathFind, bool recursive = false);
    WH_COMMON_API bool FindDirectory(std::string_view findName, std::string const& pathFind, bool recursive = false);
    WH_COMMON_API void FillFileList(std::vector<std::string>& pathList, std::string const& pathFill, bool recursive = false);
    WH_COMMON_API std::string GetFileName(std::string const& filePath);
    WH_COMMON_API uint64 GetFileSize(std::string const& filePath);
}

#endif
