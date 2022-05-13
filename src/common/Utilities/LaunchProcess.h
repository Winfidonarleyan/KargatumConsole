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

#ifndef WARHEAD_LAUNCH_PROCESS_H
#define WARHEAD_LAUNCH_PROCESS_H

#include "Define.h"
#include <string_view>

namespace Warhead::Process
{
    WH_COMMON_API void SendCommand(std::string_view path, std::string_view command);
}

namespace Warhead::Process::Git
{
    WH_COMMON_API void CommitAllFiles(std::string_view path, std::string_view commitMessage);
    WH_COMMON_API void CreateBranch(std::string_view path, std::string_view branchName);
    WH_COMMON_API void Clone(std::string_view path, std::string_view url);
    WH_COMMON_API bool IsCorrectUpstream(std::string_view path, std::string_view upstreamName);
    WH_COMMON_API void CheckoutPR(std::string_view path, std::string_view upstreamName, std::string_view prID);
    WH_COMMON_API bool IsExistBranch(std::string_view path, std::string_view name);
    WH_COMMON_API void Checkout(std::string_view path, std::string_view branchName);
}

#endif
