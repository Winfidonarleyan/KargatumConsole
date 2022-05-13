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

#include "Common.h"
#include "Cleanup.h"
#include "GitRevision.h"
#include "Log.h"
#include "StringConvert.h"
#include <iostream>
#include <tuple>
#include <memory>

void SelectCleanup()
{
    LOG_INFO("# -- Select cleanup method:");
    LOG_INFO("# 1. Remove whitespace");
    LOG_INFO("# 2. Replace tabs");
    LOG_INFO("# 3. Sort includes");
    LOG_INFO("# 4. Same includes");
    LOG_INFO("# 5. Clean ENABLE_EXTRA_LOGS");
    LOG_INFO("# 6. Check using includes");
    LOG_INFO("# 7. Replace logging format");
    LOG_INFO("# 8. Rename files (Manager - Mgr)");
    LOG_INFO("# 9. Check SHA1");
    LOG_INFO("# 10. Correct SQL variables");
    LOG_INFO("# --");
    /*
    LOG_INFO("# 10. Check bool configs");
    LOG_INFO("# 11. Check uint32 configs");
    LOG_INFO("# 12. Check int32 configs");
    LOG_INFO("# 13. Check std::string configs");
    LOG_INFO("# 14. Check float configs");
    LOG_INFO("# 15. Replace config options API");
    */
    LOG_INFO("# --");
    LOG_INFO("# 99. Exit");
    LOG_INFO("# --");
    LOG_INFO("> Select:");

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
            sClean->CheckExtraLogs();
            break;
        case 6:
            sClean->CheckUsingIncludesCount();
            break;
        case 7:
            sClean->ReplaceLoggingFormat();
            break;
        case 8:
            sClean->RenameFiles();
            break;
        case 9:
            sClean->CheckSha1DBFiles();
            break;
        case 10:
            sClean->CorrectDBFiles();
            break;
        // Configs
        /*case 10:
            sClean->CheckConfigOptions("bool");
            break;
        case 11:
            sClean->CheckConfigOptions("uint32");
            break;
        case 12:
            sClean->CheckConfigOptions("int32");
            break;
        case 13:
            sClean->CheckConfigOptions("std::string");
            break;
        case 14:
            sClean->CheckConfigOptions("float");
            break;*/
        case 15:
            sClean->ReplaceConfigOptions();
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
    LOG_INFO("> {}", GitRevision::GetFullVersion());
    LOG_INFO("--");

    sClean->LoadPathInfo();

    while (true)
    {
        SelectCleanup();
    }

    return 0;
}
