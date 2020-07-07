/*
 * Copyright (C) 2020 Winfidonarleyan
 */

#include "Common.h"
#include "Log.h"
#include "ModuleCreator.h"
#include "Poco/File.h"
#include <iostream>
#include <cstdlib>

void Load()
{
    sLog->SetLogLevel(LOG_LEVEL_DEBUG);

    LOG_INFO("> Set log level to %u", (uint8)LOG_LEVEL_DEBUG);
    LOG_INFO("");
}

int Selection()
{
    LOG_INFO("--------------------------");
    LOG_INFO("-- Select option:");
    LOG_INFO("1. Create module WarheadCore");
    LOG_INFO("2. Create module AzerothCore");
    LOG_INFO("9. Exit");
    LOG_INFO("--------------------------");
    LOG_INFO("");
    LOG_INFO("-- Enter select:");

    std::string selection;
    std::getline(std::cin, selection);
    uint32 sel = atoi(selection.c_str());

    if (sel && sel <= 2)
    {
        LOG_INFO("-- Enter %sCore module name:", sel == 1 ? "Warhead" : "Azeroth");

        std::string moduleName;
        std::getline(std::cin, moduleName);

        sModule->CreateModule(moduleName, sel == 1 ? true : false);
    }

    if (!sel || sel > 2)
        exit(0);

    return 1;
}

int main()
{
    Load();

    while (Selection())
    {
        Selection();
    }
    
    LOG_INFO("> Good bye!!!");

    return 0;
}
