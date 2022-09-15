/*
 * Copyright (C) since 2020 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan
 * Licence MIT https://opensource.org/MIT
 */

#include "ModuleCreator.h"
#include "Log.h"
#include "StringFormat.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace
{
    bool MakeDir(std::string_view path)
    {
        if (fs::exists(path))
            return false;

        try
        {
            fs::create_directory(path);
        }
        catch (fs::filesystem_error const& error)
        {
            LOG_FATAL("mc", "> Error at create module dirs '{}': {}", path, error.what());
            return false;
        }

        return true;
    }

    inline void AddLineInText(std::string& text, std::string_view fmt)
    {
        text += std::string{ fmt } + "\n";
    }

    template<typename... Args>
    inline void AddLineInText(std::string& text, std::string_view fmt, Args&&... args)
    {
        AddLineInText(text, Warhead::StringFormat(fmt, std::forward<Args>(args)...));
    }
}

ModuleCreator* ModuleCreator::instance()
{
    static ModuleCreator instance;
    return &instance;
}

bool ModuleCreator::CreateBaseArgs(std::string_view moduleName)
{
    std::string pathToModules{ "./" };

    scriptName = GetScriptsName(moduleName);
    correctModuleName = GetCorrectModuleName(moduleName);
    correctSCModuleName = GetCorrectModuleName(moduleName, true);
    pathToModule = pathToModules + correctModuleName;
    pathToModuleSrc = pathToModule + "/src/";
    defineText = GetDefineText(moduleName);

    LOG_DEBUG("mc", "");
    LOG_DEBUG("mc", "Create defines:");
    LOG_DEBUG("mc", "> scriptName - {}", scriptName);
    LOG_DEBUG("mc", "> correctModuleName - {}", correctModuleName);
    LOG_DEBUG("mc", "> pathToModule - {}", pathToModule);
    LOG_DEBUG("mc", "> pathToModuleSrc - {}", pathToModuleSrc);
    LOG_DEBUG("mc", "> defineText - {}", defineText);
    LOG_DEBUG("mc", "");

    if (!MakeDir(pathToModule))
    {
        LOG_FATAL("mc", "> Module '{}' is exist!", correctModuleName);
        return false;
    }

    return true;
}

bool ModuleCreator::AddTextInFile(std::string_view path, std::string_view text)
{
    // Path to file
    fs::path const temp(path);

    std::ofstream file(temp.generic_string());
    if (!file.is_open())
    {
        LOG_FATAL("mc", "Failed to create file '{}'", temp.generic_string());
        LOG_INFO("mc", "");
        return false;
    }

    file << text;
    file.close();

    return true;
}

bool ModuleCreator::CreateClassFiles(std::string_view moduleName)
{
    std::string cppFileName = GetScriptCPPFileName(moduleName);
    std::string hFileName = GetScriptHFileName(moduleName);

    LOG_DEBUG("mc", "> Try create cpp file - {}", cppFileName);

    std::string _text;

    AddLineInText(_text, GetHeadText());
    AddLineInText(_text, "#include \"{}\"", hFileName);
    AddLineInText(_text, "#include \"Log.h\"");
    AddLineInText(_text, "#include \"ModulesConfig.h\"");
    AddLineInText(_text, "");
    AddLineInText(_text, "{}Mgr* {}Mgr::instance()", scriptName, scriptName);
    AddLineInText(_text, "{");
    AddLineInText(_text, "    static {}Mgr instance;", scriptName);
    AddLineInText(_text, "    return &instance;");
    AddLineInText(_text, "}");
    AddLineInText(_text, "");
    AddLineInText(_text, "void {}Mgr::LoadConfig(bool /*reload*/)", scriptName);
    AddLineInText(_text, "{");
    AddLineInText(_text, "    _isEnable = MOD_CONF_GET_BOOL(\"{}.Enable\");", scriptName);
    AddLineInText(_text, "}");
    AddLineInText(_text, "");
    AddLineInText(_text, "void {}Mgr::Initialize()", scriptName);
    AddLineInText(_text, "{");
    AddLineInText(_text, "    if (!_isEnable)");
    AddLineInText(_text, "        return;");
    AddLineInText(_text, "}");
    AddLineInText(_text, "");

    if (!AddTextInFile(pathToModuleSrc + cppFileName, _text))
        return false;

    LOG_INFO("mc", "> Created - {}", cppFileName);
    LOG_DEBUG("mc", "> Try create .h file - {}", hFileName);

    _text.clear();

    AddLineInText(_text, GetHeadText(false, true));
    AddLineInText(_text, "#ifndef _{}_H_", defineText);
    AddLineInText(_text, "#define _{}_H_", defineText);
    AddLineInText(_text, "");
    AddLineInText(_text, "#include \"Define.h\"");
    AddLineInText(_text, "");
    AddLineInText(_text, "class {}Mgr", scriptName);
    AddLineInText(_text, "{");
    AddLineInText(_text, "public:");
    AddLineInText(_text, "    static {}Mgr* instance();", scriptName);
    AddLineInText(_text, "");
    AddLineInText(_text, "    void Initialize();");
    AddLineInText(_text, "    void LoadConfig(bool reload);");
    AddLineInText(_text, "");
    AddLineInText(_text, "private:");
    AddLineInText(_text, "    bool _isEnable{ false };");
    AddLineInText(_text, "");
    AddLineInText(_text, "    {}Mgr() = default;", scriptName);
    AddLineInText(_text, "    ~{}Mgr() = default;", scriptName);
    AddLineInText(_text, "");
    AddLineInText(_text, "    {}Mgr({}Mgr const&) = delete;", scriptName, scriptName);
    AddLineInText(_text, "    {}Mgr({}Mgr&&) = delete;", scriptName, scriptName);
    AddLineInText(_text, "    {}Mgr& operator=({}Mgr const&) = delete;", scriptName, scriptName);
    AddLineInText(_text, "    {}Mgr& operator=({}Mgr&&) = delete;", scriptName, scriptName);
    AddLineInText(_text, "");
    AddLineInText(_text, "};");
    AddLineInText(_text, "");
    AddLineInText(_text, "#define s{}Mgr {}Mgr::instance()", scriptName, scriptName);
    AddLineInText(_text, "");
    AddLineInText(_text, "#endif // _{}_H_", defineText);

    if (!AddTextInFile(pathToModuleSrc + hFileName, _text))
        return false;

    LOG_INFO("mc", "> Created - {}", hFileName);
    return true;
}

bool ModuleCreator::CreateScriptLoader(std::string_view moduleName)
{
    std::string fileName = GetScriptLoaderFileName(moduleName);

    LOG_DEBUG("> Try create script loader - {}", fileName);

    std::string _text;

    AddLineInText(_text, GetHeadText());
    AddLineInText(_text, "// From SC");
    AddLineInText(_text, "void AddSC_{}();", scriptName);
    AddLineInText(_text, "");
    AddLineInText(_text, "// Add all");
    AddLineInText(_text, "void Add{}Scripts()", correctSCModuleName);
    AddLineInText(_text, "{}", "{");
    AddLineInText(_text, "    AddSC_{}();", scriptName);
    AddLineInText(_text, "{}", "}");

    if (!AddTextInFile(pathToModuleSrc + fileName, _text))
        return false;

    LOG_INFO("mc", "> Created script loader ({})", fileName);

    return true;
}

bool ModuleCreator::CreateSCFile(std::string_view moduleName)
{
    std::string SCFileName = GetSCCPPFileName(moduleName);

    LOG_DEBUG("mc", "> Try create cpp file - {}", SCFileName);

    std::string _text;

    AddLineInText(_text, GetHeadText());
    AddLineInText(_text, "#include \"{}\"", GetScriptHFileName(moduleName));
    AddLineInText(_text, "#include \"ScriptObject.h\"");
    AddLineInText(_text, "");
    AddLineInText(_text, "class {}_World : public WorldScript", scriptName);
    AddLineInText(_text, "{}", "{");
    AddLineInText(_text, "public:");
    AddLineInText(_text, "    {}_World() : WorldScript(\"{}_World\") {}", scriptName, scriptName, "{ }");
    AddLineInText(_text, "");
    AddLineInText(_text, "    void OnAfterConfigLoad(bool reload) override");
    AddLineInText(_text, "    {}", "{");
    AddLineInText(_text, "        s{}Mgr->LoadConfig(reload);", scriptName);
    AddLineInText(_text, "    {}", "}");
    AddLineInText(_text, "");
    AddLineInText(_text, "    void OnStartup() override");
    AddLineInText(_text, "    {}", "{");
    AddLineInText(_text, "        s{}Mgr->Initialize();", scriptName);
    AddLineInText(_text, "    {}", "}");
    AddLineInText(_text, "{};", "}");
    AddLineInText(_text, "");
    AddLineInText(_text, "// Group all custom scripts");
    AddLineInText(_text, "void AddSC_{}()", scriptName);
    AddLineInText(_text, "{}", "{");
    AddLineInText(_text, "    new {}_World();", scriptName);
    AddLineInText(_text, "{}", "}");

    if (!AddTextInFile(pathToModuleSrc + SCFileName, _text))
        return false;

    LOG_INFO("mc", "> Created - {}", SCFileName);

    return true;
}

bool ModuleCreator::CreateConfigFile(std::string_view moduleName)
{
    std::string configFileName = GetConfigFileName(moduleName);

    LOG_DEBUG("mc", "> Try create config file - {}", configFileName);

    std::string _text;

    AddLineInText(_text, GetHeadText(true));
    AddLineInText(_text, "########################################");
    AddLineInText(_text, "# {} module configuration", GetScriptsName(moduleName));
    AddLineInText(_text, "########################################");
    AddLineInText(_text, "#");
    AddLineInText(_text, "#    {}.Enable", GetScriptsName(moduleName));
    AddLineInText(_text, "#        Description: Enable this module");
    AddLineInText(_text, "#        Default: 0");
    AddLineInText(_text, "#");
    AddLineInText(_text, "");
    AddLineInText(_text, "{}.Enable = 0", GetScriptsName(moduleName));

    if (!AddTextInFile(pathToModule + "/conf/" + configFileName, _text))
        return false;

    LOG_INFO("mc", "> Created - {}", configFileName);
    return true;
}

bool ModuleCreator::CreateBaseDirs()
{
    return MakeDir(pathToModule + "/" + "conf") && MakeDir(pathToModule + "/" + "src");
}

void ModuleCreator::CreateModule(std::string_view moduleName)
{
    if (!CreateBaseArgs(moduleName))
        return;

    LOG_INFO("mc", "Creating WarheadCore module - {}", moduleName);
    LOG_INFO("mc", "> Move base module files to new module - {}", correctModuleName);

    // #1. Create base files
    CreateBaseDirs();

    // #2. Create script_loader.cpp
    CreateScriptLoader(moduleName);

    // #3. Create class files
    CreateClassFiles(moduleName);

    // #4. Create SC file
    CreateSCFile(moduleName);

    // #5. Create config file
    CreateConfigFile(moduleName);
}

// Text transform
std::string ModuleCreator::GetHeadText(bool isCmakeFile /*= false*/, bool isHeader /*= false*/)
{
    std::string text;

    if (isCmakeFile)
    {
        text += "#\n";
        text += "# This file is part of the WarheadCore Project. See AUTHORS file for Copyright information\n";
        text += "#\n";
        text += "# This program is free software; you can redistribute it and/or modify it\n";
        text += "# under the terms of the GNU Affero General Public License as published by the\n";
        text += "# Free Software Foundation; either version 3 of the License, or (at your\n";
        text += "# option) any later version.\n";
        text += "#\n";
        text += "# This program is distributed in the hope that it will be useful, but WITHOUT\n";
        text += "# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\n";
        text += "# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for\n";
        text += "# more details.\n";
        text += "#\n";
        text += "# You should have received a copy of the GNU General Public License along\n";
        text += "# with this program. If not, see <http://www.gnu.org/licenses/>.\n";
        text += "#\n";
    }
    else
    {
        text += "/*\n";
        text += " * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information\n";
        text += " *\n";
        text += " * This program is free software; you can redistribute it and/or modify it\n";
        text += " * under the terms of the GNU Affero General Public License as published by the\n";
        text += " * Free Software Foundation; either version 3 of the License, or (at your\n";
        text += " * option) any later version.\n";
        text += " *\n";
        text += " * This program is distributed in the hope that it will be useful, but WITHOUT\n";
        text += " * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\n";
        text += " * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for\n";
        text += " * more details.\n";
        text += " *\n";
        text += " * You should have received a copy of the GNU General Public License along\n";
        text += " * with this program. If not, see <http://www.gnu.org/licenses/>.\n";
        text += " */\n";

        if (!isHeader)
        {
            text += "\n";
            text += "// This is an open source non-commercial project. Dear PVS-Studio, please check it.\n";
            text += "// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com\n";
        }
    }

    return text;
}

std::string ModuleCreator::GetScriptsName(std::string_view moduleName)
{
    std::string str{ moduleName };
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str;
}

std::string ModuleCreator::GetConfigFileName(std::string_view moduleName)
{
    std::string str{ moduleName };
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".conf.dist";
}

std::string ModuleCreator::GetScriptLoaderFileName(std::string_view moduleName)
{
    std::string text;
    std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(text), tolower);
    std::replace(text.begin(), text.end(), '-', '_');
    return text + "_loader.cpp";
}

std::string ModuleCreator::GetScriptCPPFileName(std::string_view moduleName)
{
    std::string str{ moduleName };
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".cpp";
}

std::string ModuleCreator::GetSCCPPFileName(std::string_view moduleName)
{
    std::string str{ moduleName };
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + "_SC.cpp";
}

std::string ModuleCreator::GetScriptHFileName(std::string_view moduleName)
{
    std::string str{ moduleName };
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".h";
}

std::string ModuleCreator::GetCorrectModuleName(std::string_view moduleName, bool sc /*= false*/)
{
    std::string text;
    std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(text), tolower);

    if (sc)
        std::replace(text.begin(), text.end(), '-', '_');

    return sc ? "mod_" + text : "mod-" + text;
}

std::string ModuleCreator::GetDefineText(std::string_view moduleName)
{
    std::string text;
    std::transform(moduleName.begin(), moduleName.end(), std::back_inserter(text), toupper);
    std::replace(text.begin(), text.end(), '-', '_');
    return text;
}
