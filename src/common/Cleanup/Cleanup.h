/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
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

#ifndef _CLEANUP_H_
#define _CLEANUP_H_

#include "Common.h"
#include <string_view>

class WH_COMMON_API Cleanup
{
public:
    static Cleanup* instance();

    void RemoveWhitespace();
    void ReplaceTabs();
    void SortIncludes();

    void SendPathInfo();

    bool SetPath(std::string const& path);
    void CleanPath();
    std::string const GetPath();
    bool IsCorrectPath();
    uint32 GetFoundFiles();
};

#define sClean Cleanup::instance()

#endif // _CLEANUP_H_
