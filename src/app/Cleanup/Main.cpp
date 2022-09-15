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

#include "Cleanup.h"
#include "Config.h"
#include "GitRevision.h"
#include "Log.h"
#include "StringConvert.h"
#include <iostream>

void SelectCleanup()
{
    LOG_INFO("cleanup", "# -- Select cleanup method:");
    LOG_INFO("cleanup", "# 1. Remove whitespace");
    LOG_INFO("cleanup", "# 2. Replace tabs");
    LOG_INFO("cleanup", "# 3. Sort includes");
    LOG_INFO("cleanup", "# 4. Same includes");
    LOG_INFO("cleanup", "# 5. Check using includes");
    LOG_INFO("cleanup", "# --");
    LOG_INFO("cleanup", "# 99. Exit");
    LOG_INFO("cleanup", "# --");
    LOG_INFO("cleanup", "> Select:");

    std::string selOptionCleanup;
    std::getline(std::cin, selOptionCleanup);
    uint32 cleanupMethod = *Warhead::StringTo<uint32>(selOptionCleanup);

    switch (cleanupMethod)
    {
        case 1:
            sClean->RemoveWhitespace();
            break;
        case 2:
            sClean->ReplaceTabs();
            break;
        case 3:
            sClean->SortIncludes();
            break;
        case 4:
            sClean->CheckSameIncludes();
            break;
        case 5:
            sClean->CheckUsingIncludesCount();
            break;
        // Exit console
        case 99:
            exit(0);
            break;
        default:
            SelectCleanup();
            break;
    }
}

int main()
{
    sLog->UsingDefaultLogs();

    LOG_INFO("cleanup", "> {}", GitRevision::GetFullVersion());

    sClean->LoadPathInfo();

    while (true)
    {
        SelectCleanup();
    }

    return 0;
}
