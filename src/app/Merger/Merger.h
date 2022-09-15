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

#ifndef _MERGER_H_
#define _MERGER_H_

#include "Define.h"
#include <string>

class WH_COMMON_API Merger
{
public:
    static Merger* instance();

    void Init();

    void SendPathInfo();
    void LoadPathInfo();

    bool SetPath(std::string const& path);
    void CleanPath();
    std::string const GetPath();
    bool IsCorrectPath();
    uint32 GetFoundFiles();
};

#define sMerger Merger::instance()

#endif // _MERGER_H_
