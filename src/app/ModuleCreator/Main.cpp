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
#include "Log.h"
#include "ModuleCreator.h"
#include "StringConvert.h"
#include "GitRevision.h"
#include <iostream>

void Load()
{
    sLog->SetLogLevel(LOG_LEVEL_DEBUG);
}

void Selection();

void SelectCreateModule(uint32 option)
{
    system("cls");
    LOG_INFO("-- Enter %sCore module name:", option == 1 ? "Warhead" : "Azeroth");

    std::string moduleName;
    std::getline(std::cin, moduleName);

    sModule->CreateModule(moduleName, option == 1 ? true : false);

    Selection();
}

void Selection()
{
    LOG_INFO("%s", GitRevision::GetFullVersion());
    LOG_INFO("");
    LOG_INFO("# -- Select option:");
    LOG_INFO("1. Create module WarheadCore");
    LOG_INFO("2. Create module AzerothCore");
    LOG_INFO("# --");
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
        Selection();
    }

    return 0;
}
