/*
 * Copyright (C) 2020 Winfidonarleyan
 */

#include "Common.h"
#include "Log.h"
#include "ModuleCreator.h"
#include <iostream>

void Load()
{
    sLog->SetLogLevel(LOG_LEVEL_DEBUG);

    LOG_INFO("Set log level to %u", (uint8)LOG_LEVEL_DEBUG);
    LOG_INFO("");
}

int Selection()
{
    LOG_INFO("--------------------------");
    LOG_INFO("-- Select option:");
    LOG_INFO("1. Create module");
    LOG_INFO("2. Exit");
    LOG_INFO("--------------------------");
    LOG_INFO("");
    LOG_INFO("-- Enter select:");

    std::string selection;
    std::getline(std::cin, selection);

    if (selection == "1")
    {
        LOG_INFO("-- Enter module name:");

        std::getline(std::cin, selection);
        sModule->CreateModule(selection);
    }

    if (selection.empty() || selection == "2")
        return 0;

    return 1;
}

int main()
{
    Load();

    LOG_INFO("реяр TEST");

    while (Selection())
    {
        Selection();
    }

    //std::string text = "Kargatum_Anti_AD";

    //LOG_INFO("Kargatum_Anti_AD -> %s", TransformToModuleName(text));

    LOG_INFO("> Good bye!!!");

    return 0;
}