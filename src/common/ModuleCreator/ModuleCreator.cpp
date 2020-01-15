/*
 * Copyright (C) 
 */

#include "ModuleCreator.h"
#include "Log.h"
#include "StringFormat.h"
#include <filesystem>
#include <fstream>
#include <iostream>

ModuleCreator* ModuleCreator::instance()
{
    static ModuleCreator instance;
    return &instance;
}

void ModuleCreator::CreateBaseArgs(std::string const& moduleName)
{
    scriptName = GetScriptName(moduleName);
    correctModuleName = GetCorrectModuleName(moduleName);
    pathToModule = "mods-created/" + correctModuleName;
    defineText = GetUpperText(moduleName);
}

std::string ModuleCreator::GetHeadText()
{
    std::string text;
    
    text += "/*";
    text += " * Copyright (C) since 2020 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan";
    text += " * Licence MIT https://opensource.org/MIT";
    text += " */";

    return text;
}

bool ModuleCreator::CreateScriptLoader(std::string const& moduleName)
{
    std::string fileName = GetScriptLoaderFileName(moduleName);

    // Path of file
    std::filesystem::path const temp(pathToModule + "/src/" + fileName);

    LOG_DEBUG("> Try create script loader - %s", fileName.c_str());

    std::ofstream file(temp.generic_string());
    if (!file.is_open())
    {
        LOG_FATAL("Failed to create script loader \"%s\"!", temp.generic_string().c_str());
        LOG_INFO("");
        return false;
    }

    std::string _text;

    AddLineInText(_text, GetHeadText().c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "#ifndef _%s_LOADER_H_", defineText.c_str());
    AddLineInText(_text, "#define _%s_LOADER_H_", defineText.c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "// From SC");
    AddLineInText(_text, "void AddSC_%s();", scriptName.c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "// Add all");
    AddLineInText(_text, "void Add%sScripts()", scriptName.c_str());
    AddLineInText(_text, "{");
    AddLineInText(_text, "   AddSC_%s();", scriptName.c_str());
    AddLineInText(_text, "}");
    AddLineInText(_text, "");
    AddLineInText(_text, "#endif /* %s */", defineText.c_str());

    file.close();

    LOG_DEBUG("> Created script loader (%s)", temp.generic_string().c_str());

    return true;
}

bool ModuleCreator::CopyBaseModuleFiles()
{
    namespace fs = std::filesystem;
    std::error_code e;

    // Copy base module files
    fs::copy("mod-based", pathToModule, fs::copy_options::recursive, e);

    if (e.value())
    {
        LOG_ERROR("> Failed copy modules base files - %s", e.message().c_str());
        return false;
    }

    return true;
}

void ModuleCreator::CreateModule(std::string const& moduleName)
{
    LOG_INFO("Creating module - %s", moduleName.c_str());
    LOG_INFO("> Move base module files to new module - %s", correctModuleName.c_str());

    CopyBaseModuleFiles();
    //CreateScriptLoader(moduleName);
}

void ModuleCreator::AddLineInText(std::string& text, std::string&& message)
{
    text += message + "\n";

    LOG_TRACE("> Added line (%s)", message.c_str());
}

std::string ModuleCreator::GetScriptName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    return str;
}

std::string ModuleCreator::GetScriptLoaderFileName(std::string str)
{
    std::string text;
    std::transform(str.begin(), str.end(), std::back_inserter(text), tolower);
    return text + "_loader.cpp";
}

std::string ModuleCreator::GetCorrectModuleName(std::string str)
{
    std::string text;
    std::transform(str.begin(), str.end(), std::back_inserter(text), tolower);
    std::replace(text.begin(), text.end(), '_', '-');

    return "mod-" + text;
}

std::string ModuleCreator::GetUpperText(std::string str)
{
    std::string text;
    std::transform(str.begin(), str.end(), std::back_inserter(text), toupper);
    return text;
}