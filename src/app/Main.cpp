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

#include "Common.h"
#include "ModuleCreator.h"
#include "StringConvert.h"
#include "StringFormat.h"
#include "Cleanup.h"
#include "Timer.h"
#include "Log.h"
#include "Util.h"
#include "GitRevision.h"
#include <iostream>
#include <fstream>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "Poco/String.h"

void Load()
{
    sLog->SetLogLevel(LOG_LEVEL_DEBUG);
}

void _Selection();

void SelectCreateModule(uint32 option)
{
    system("cls");
    LOG_INFO("-- Enter %sCore module name:", option == 1 ? "Warhead" : "Azeroth");

    std::string moduleName;
    std::getline(std::cin, moduleName);

    sModule->CreateModule(moduleName, option == 1 ? true : false);

    _Selection();
}

void SelectCleanup()
{
    system("cls");

    LOG_INFO("# -- Select cleanup method:");
    LOG_INFO("# 1. Remove whitespace");
    LOG_INFO("# 2. Replace tabs");
    LOG_INFO("# 3. Sort includes (with check first include)");
    LOG_INFO("# 4. Sort includes (without check first include");
    LOG_INFO("# --");
    LOG_INFO("# 9. To main menu");
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
            sClean->SortIncludes(false);
            break;
        case 9:
            _Selection();
            break;
        default:
            SelectCleanup();
            break;
    }
}

void _Selection()
{
    LOG_INFO("%s", GitRevision::GetFullVersion());
    LOG_INFO("");
    LOG_INFO("# -- Select option:");
    LOG_INFO("1. Create module WarheadCore");
    LOG_INFO("2. Create module AzerothCore");
    LOG_INFO("3. Cleanup");
    LOG_INFO("9. Exit");
    LOG_INFO("# --");
    LOG_INFO("> Enter select:");

    std::string selection;
    std::getline(std::cin, selection);
    uint32 option = *Warhead::StringTo<uint32>(selection);

    switch (option)
    {
        case 1:
        case 2:
            SelectCreateModule(option);
            break;
        case 3:
            SelectCleanup();
            break;
        case 9:
            exit(0);
        default:
            break;
    }
}

int main()
{
    Load();

    while (true)
    {
        _Selection();
    }

    return 0;
}
