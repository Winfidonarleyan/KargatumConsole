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
#include <string>
#include <string_view>
#include <utility>
#include <vector>

WH_COMMON_API bool StringEqualI(std::string_view str1, std::string_view str2);

// UTF8 handling
WH_COMMON_API bool Utf8toWStr(std::string_view utf8str, std::wstring& wstr);

namespace Warhead::File
{
    WH_COMMON_API std::string GetFileText(std::string const& path, bool openBinary = false);
    WH_COMMON_API bool FindFile(std::string_view findName, std::string const& pathFind, bool recursive = false);
    WH_COMMON_API bool FindDirectory(std::string_view findName, std::string const& pathFind, bool recursive = false);
    WH_COMMON_API void FillFileList(std::vector<std::string>& pathList, std::string const& pathFill, bool recursive = false);
    WH_COMMON_API std::string GetFileName(std::string const& filePath);
    WH_COMMON_API uint64 GetFileSize(std::string const& filePath);
}

namespace Warhead::Impl
{
    std::string ByteArrayToHexStr(uint8 const* bytes, size_t length, bool reverse = false);
    void HexStrToByteArray(std::string const& str, uint8* out, size_t outlen, bool reverse = false);
}

template<typename Container>
std::string ByteArrayToHexStr(Container const& c, bool reverse = false)
{
    return Warhead::Impl::ByteArrayToHexStr(std::data(c), std::size(c), reverse);
}

#endif
