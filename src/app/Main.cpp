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
#include "Log.h"
#include "ModuleCreator.h"
#include "StringConvert.h"
#include "Cleanup.h"
#include "Timer.h"
#include "GitRevision.h"
#include <iostream>

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

void SelectCleanup(bool needCleanPath = false)
{
    // Clean path before enter
    if (needCleanPath)
        sClean->CleanPath();

    system("cls");

    auto SendEmptyPath = []()
    {
        auto GetPathFromConsole = [](uint32 selectOption)
        {
            std::string whitespacePath;

            switch (selectOption)
            {
                case 1:
                    whitespacePath = "F:\\Git\\Warhead\\src\\server\\scripts\\Warhead";
                    break;
                case 2:
                    whitespacePath = "F:\\Git\\Warhead\\src\\server\\scripts";
                    break;
                case 3:
                    whitespacePath = "F:\\Git\\Warhead\\src\\server\\game";
                    break;
                case 4:
                    whitespacePath = "F:\\Git\\Warhead\\src\\server";
                    break;
                case 5:
                    whitespacePath = "F:\\Git\\Warhead\\src";
                    break;
                case 6:
                    whitespacePath = "F:\\Git\\Warhead\\cmake";
                    break;
                case 8:
                    LOG_INFO("-- Enter path:");
                    std::getline(std::cin, whitespacePath);
                    break;
                default:
                    SelectCleanup();
                    break;
            }

            return whitespacePath;
        };

        LOG_FATAL(">> Path is empty! Please enter path.");
        LOG_INFO("# -- Warhead");
        LOG_INFO("1. ./src/server/scripts/Warhead");
        LOG_INFO("2. ./src/server/scripts");
        LOG_INFO("3. ./src/server/game");
        LOG_INFO("4. ./src/server");
        LOG_INFO("5. ./src");
        LOG_INFO("6. ./cmake");
        LOG_INFO("# --");
        LOG_INFO("8. Enter path manually");
        LOG_INFO("> Select:");

        std::string selPathEnter;
        std::getline(std::cin, selPathEnter);

        if (!sClean->SetPath(GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter))))
            SelectCleanup();
    };

    std::string cleanPath = sClean->GetPath();
    if (cleanPath.empty())
        SendEmptyPath();

    // Refresh info
    cleanPath = sClean->GetPath();

    if (!sClean->IsCorrectPath())
        SelectCleanup();

    system("cls");

    uint32 filesFound = sClean->GetFoundFiles();

    LOG_INFO(">> Entered path '%s'", cleanPath.c_str());
    LOG_INFO(">> Files found '%u'", filesFound);
    LOG_INFO("# --");
    LOG_INFO("# -- Select cleanup method:");
    LOG_INFO("# 1. Replace whitespace");
    LOG_INFO("# 2. Replace tabs");
    LOG_INFO("# --");
    LOG_INFO("# -- Select path functions:");
    LOG_INFO("# 3. Replace path");
    LOG_INFO("# --");
    LOG_INFO("# -- Select other functions:");
    LOG_INFO("# 9. To main menu");
    LOG_INFO("# --");
    LOG_INFO("> Select:");

    std::string selOptionCleanup;
    std::getline(std::cin, selOptionCleanup);
    uint32 cleanupMethod = *Warhead::StringTo<uint32>(selOptionCleanup);

    switch (cleanupMethod)
    {
        case 1:
            sClean->CheckWhitespace();
            break;
        case 2:
            sClean->CheckTabs();
            break;
        case 3:
            SelectCleanup(true);
            break;
        case 8:
            SelectCleanup();
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
            SelectCleanup(true);
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
