/*
 * Copyright (C) since 2020 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan
 * Licence MIT https://opensource.org/MIT
 */

#include "ModuleCreator.h"
#include "Log.h"
#include "Poco/Exception.h"
#include "Poco/File.h"
#include "StringFormat.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
    void CheckBaseDir()
    {
        Poco::File dir("mods-created");

        if (dir.exists())
            return;

        try
        {
            dir.createDirectory();
        }
        catch (const Poco::Exception& e)
        {
            LOG_FATAL("> Failed create directory: %s", e.displayText().c_str());
        }
    }

    bool MakeDirForNewModule(std::string const& path)
    {
        Poco::File dir(path);

        if (dir.exists())
            return false;

        try
        {
            dir.createDirectories();
        }
        catch (const Poco::Exception& e)
        {
            LOG_FATAL("> Failed create directory: %s", e.displayText().c_str());
        }

        return true;
    }
}

ModuleCreator* ModuleCreator::instance()
{
    static ModuleCreator instance;
    return &instance;
}

bool ModuleCreator::CreateBaseArgs(std::string const& moduleName)
{
    ::CheckBaseDir();

    scriptName = GetScriptsName(moduleName);
    correctModuleName = GetCorrectModuleName(moduleName);
    pathToModule = "mods-created/" + correctModuleName;
    pathToModuleSrc = pathToModule + "/src/";
    defineText = GetUpperText(moduleName);

    LOG_DEBUG("");
    LOG_DEBUG("Create defines:");
    LOG_DEBUG("> scriptName - %s", scriptName.c_str());
    LOG_DEBUG("> correctModuleName - %s", correctModuleName.c_str());
    LOG_DEBUG("> pathToModule - %s", pathToModule.c_str());
    LOG_DEBUG("> pathToModuleSrc - %s", pathToModuleSrc.c_str());
    LOG_DEBUG("> defineText - %s", defineText.c_str());
    LOG_DEBUG("");

    if (!MakeDirForNewModule(pathToModule))
    {
        LOG_FATAL("> Module '%s' is exist!", correctModuleName.c_str());
        return false;
    }

    return true;
}

void ModuleCreator::AddLineInText(std::string& text, std::string&& message)
{
    text += message + "\n";

    LOG_TRACE("> Added line (%s)", message.c_str());
}

bool ModuleCreator::AddTextInFile(std::string const& path, std::string const& text)
{
    // Path to file
    std::filesystem::path const temp(path);

    std::ofstream file(temp.generic_string());
    if (!file.is_open())
    {
        LOG_FATAL("Failed to create file '%s'", temp.generic_string().c_str());
        LOG_INFO("");
        return false;
    }

    file << text;
    file.close();

    return true;
}

bool ModuleCreator::CreateClassFiles(std::string const& moduleName)
{
    std::string cppFileName = GetScriptCPPFileName(moduleName);
    std::string hFileName = GetScriptHFileName(moduleName);

    LOG_DEBUG("> Try create cpp file - %s", cppFileName.c_str());

    std::string _text;

    AddLineInText(_text, GetHeadText().c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "#include \"%s\"", hFileName.c_str());
    AddLineInText(_text, "#include \"Log.h\"");
    AddLineInText(_text, "#include \"%s.h\"", _IsWarheadModule ? "GameConfig" : "Config");

    if (!AddTextInFile(pathToModuleSrc + cppFileName, _text))
        return false;

    LOG_INFO("> Created - %s", cppFileName.c_str());
    LOG_DEBUG("> Try create h file - %s", hFileName.c_str());

    _text.clear();

    AddLineInText(_text, GetHeadText().c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "#ifndef _%s_H_", defineText.c_str());
    AddLineInText(_text, "#define _%s_H_", defineText.c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "#endif /* _%s_H_ */", defineText.c_str());

    if (!AddTextInFile(pathToModuleSrc + hFileName, _text))
        return false;

    LOG_INFO("> Created - %s", hFileName.c_str());

    return true;
}

bool ModuleCreator::CreateScriptLoader(std::string const& moduleName)
{
    std::string fileName = GetScriptLoaderFileName(moduleName);

    LOG_DEBUG("> Try create script loader - %s", fileName.c_str());

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
    AddLineInText(_text, "#endif /* _%s_LOADER_H_ */", defineText.c_str());

    if (!AddTextInFile(pathToModuleSrc + fileName, _text))
        return false;

    LOG_INFO("> Created script loader (%s)", fileName.c_str());

    return true;
}

bool ModuleCreator::CreateSCFile(std::string const& moduleName)
{
    std::string SCFileName = GetSCCPPFileName(moduleName);
    std::string hFileName = GetScriptHFileName(moduleName);

    LOG_DEBUG("> Try create cpp file - %s", SCFileName.c_str());

    std::string _text;

    AddLineInText(_text, GetHeadText().c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "#include \"%s\"", hFileName.c_str());
    AddLineInText(_text, "#include \"Log.h\"");
    AddLineInText(_text, "#include \"ScriptMgr.h\"");
    AddLineInText(_text, "#include \"%s.h\"", _IsWarheadModule ? "GameConfig" : "Config");
    AddLineInText(_text, "#include \"Chat.h\"");
    AddLineInText(_text, "#include \"Player.h\"");
    AddLineInText(_text, "#include \"ScriptedGossip.h\"");
    AddLineInText(_text, "");
    AddLineInText(_text, "class %s_World : public WorldScript", scriptName.c_str());
    AddLineInText(_text, "{");
    AddLineInText(_text, "public:");
    AddLineInText(_text, "    %s_World() : WorldScript(\"%s_World\") { }", scriptName.c_str(), scriptName.c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "    void OnAfterConfigLoad(bool /*reload*/) override");
    AddLineInText(_text, "    {");
    AddLineInText(_text, "        // Add conigs options configiration");
    AddLineInText(_text, "    }");
    AddLineInText(_text, "};");
    AddLineInText(_text, "");
    AddLineInText(_text, "// Group all custom scripts");
    AddLineInText(_text, "void AddSC_%s()", scriptName.c_str());
    AddLineInText(_text, "{");
    AddLineInText(_text, "    new %s_World();", scriptName.c_str());
    AddLineInText(_text, "}");

    if (!AddTextInFile(pathToModuleSrc + SCFileName, _text))
        return false;

    LOG_INFO("> Created - %s", SCFileName.c_str());

    return true;
}

bool ModuleCreator::CreateConfigFile(std::string const& moduleName)
{
    std::string configFileName = GetConfigFileName(moduleName);

    LOG_DEBUG("> Try create config file - %s", configFileName.c_str());

    std::string _text;

    AddLineInText(_text, GetHeadText(true).c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "########################################");
    AddLineInText(_text, "# %s module configuration", GetScriptsName(moduleName).c_str());
    AddLineInText(_text, "########################################");
    AddLineInText(_text, "[worldserver]");
    AddLineInText(_text, "");
    AddLineInText(_text, "#");
    AddLineInText(_text, "#");
    AddLineInText(_text, "#");

    if (!AddTextInFile(pathToModule + "/conf/" + configFileName, _text))
        return false;

    LOG_INFO("> Created - %s", configFileName.c_str());

    return true;
}

bool ModuleCreator::CreateCmakeFile(std::string const& moduleName)
{
    LOG_DEBUG("> Try create - CMakeLists.txt");

    std::string _text;

    AddLineInText(_text, GetHeadText(true).c_str());
    AddLineInText(_text, "");
    AddLineInText(_text, "# Add source files");

    if (_IsWarheadModule)
        AddLineInText(_text, "WH_ADD_MODULES_SOURCE(\"%s\")", scriptName.c_str());
    else
    {
        AddLineInText(_text, "AC_ADD_SCRIPT(\"${CMAKE_CURRENT_LIST_DIR}/src/%s\")", GetSCCPPFileName(moduleName).c_str());
        AddLineInText(_text, "");
        AddLineInText(_text, "# Add scripts to script loader");
        AddLineInText(_text, "AC_ADD_SCRIPT_LOADER(\"%s\" \"${CMAKE_CURRENT_LIST_DIR}/src/%s\")", scriptName.c_str(), GetScriptLoaderFileName(moduleName).c_str());
    }

    AddLineInText(_text, "");
    AddLineInText(_text, "# Add config file");
    AddLineInText(_text, "%s_ADD_CONFIG_FILE(\"${CMAKE_CURRENT_LIST_DIR}/conf/%s\")", _IsWarheadModule ? "WH" : "AC", GetConfigFileName(moduleName).c_str());

    if (!AddTextInFile(pathToModule + "/" + "CMakeLists.txt", _text))
        return false;

    LOG_INFO("> Created - CMakeLists.txt");

    return true;
}

bool ModuleCreator::CreateBaseFiles()
{
    // .editorconfig
    std::string _text = "[*]\n"
                        "charset = utf-8\n"
                        "indent_style = space\n"
                        "indent_size = 4\n"
                        "tab_width = 4\n"
                        "insert_final_newline = true\n"
                        "trim_trailing_whitespace = true\n"
                        "max_line_length = 80\n";

    if (!AddTextInFile(pathToModule + "/" + ".editorconfig", _text))
        return false;

    // .gitignore
    _text = "!.gitignore\n"
            "\n"
            "#\n"
            "#Generic\n"
            "#"
            "\n"
            ".directory\n"
            ".mailmap\n"
            "* .orig\n"
            "* .rej\n"
            "* .*~\n"
            ".hg /\n"
            "*.kdev *\n"
            ".DS_Store\n"
            "CMakeLists.txt.user\n"
            "* .bak\n"
            "* .patch\n"
            "* .diff\n"
            "* .REMOTE.*\n"
            "* .BACKUP.*\n"
            "* .BASE.*\n"
            "* .LOCAL.*\n"
            "\n"
            "#\n"
            "# IDE & other softwares\n"
            "#\n"
            "/ .settings/\n"
            "/.externalToolBuilders/*\n"
            "# exclude in all levels\n"
            "nbproject/\n"
            ".sync.ffs_db\n"
            "*.kate-swp\n"
            "\n"
            "#\n"
            "# Eclipse\n"
            "#\n"
            "*.pydevproject\n"
            ".metadata\n"
            ".gradle\n"
            "tmp/\n"
            "*.tmp\n"
            "*.swp\n"
            "*~.nib\n"
            "local.properties\n"
            ".settings/\n"
            ".loadpath\n"
            ".project\n"
            ".cproject\n";

    if (!AddTextInFile(pathToModule + "/" + ".gitignore", _text))
        return false;

    // .gitattributes
    _text = "## AUTO-DETECT\n"
            "##   Handle line endings automatically for files detected as\n"
            "##   text and leave all files detected as binary untouched.\n"
            "##   This will handle all files NOT defined below.\n"
            "* text = auto eol = lf\n"
            "\n"
            "# Text\n"
            "* .conf text\n"
            "* .conf.dist text\n"
            "* .cmake text\n"
            "\n"
            "## Scripts\n"
            "* .sh text\n"
            "* .fish text\n"
            "* .lua text\n"
            "\n"
            "## SQL\n"
            "* .sql text\n"
            "\n"
            "## C++\n"
            "* .c text\n"
            "* .cc text\n"
            "* .cxx text\n"
            "* .cpp text\n"
            "* .c++ text\n"
            "* .hpp text\n"
            "* .h text\n"
            "* .h++ text\n"
            "* .hh text\n"
            "\n"
            "\n"
            "## For documentation\n"
            "\n"
            "# Documents\n"
            "* .doc	 diff = astextplain\n"
            "* .DOC	 diff = astextplain\n"
            "* .docx diff = astextplain\n"
            "* .DOCX diff = astextplain\n"
            "* .dot  diff = astextplain\n"
            "* .DOT  diff = astextplain\n"
            "* .pdf  diff = astextplain\n"
            "* .PDF	 diff = astextplain\n"
            "* .rtf	 diff = astextplain\n"
            "* .RTF	 diff = astextplain\n"
            "\n"
            "## DOCUMENTATION\n"
            "* .markdown   text\n"
            "* .md         text\n"
            "* .mdwn       text\n"
            "* .mdown      text\n"
            "* .mkd        text\n"
            "* .mkdn       text\n"
            "* .mdtxt      text\n"
            "* .mdtext     text\n"
            "* .txt        text\n"
            "AUTHORS      text\n"
            "CHANGELOG    text\n"
            "CHANGES      text\n"
            "CONTRIBUTING text\n"
            "COPYING      text\n"
            "copyright    text\n"
            "* COPYRIGHT * text\n"
            "INSTALL      text\n"
            "license      text\n"
            "LICENSE      text\n"
            "NEWS         text\n"
            "readme       text\n"
            "* README * text\n"
            "TODO         text\n"
            "\n"
            "## GRAPHICS\n"
            "* .ai   binary\n"
            "* .bmp  binary\n"
            "* .eps  binary\n"
            "* .gif  binary\n"
            "* .ico  binary\n"
            "* .jng  binary\n"
            "* .jp2  binary\n"
            "* .jpg  binary\n"
            "* .jpeg binary\n"
            "* .jpx  binary\n"
            "* .jxr  binary\n"
            "* .pdf  binary\n"
            "* .png  binary\n"
            "* .psb  binary\n"
            "* .psd  binary\n"
            "* .svg  text\n"
            "* .svgz binary\n"
            "* .tif  binary\n"
            "* .tiff binary\n"
            "* .wbmp binary\n"
            "* .webp binary\n"
            "\n"
            "\n"
            "## ARCHIVES\n"
            "* .7z  binary\n"
            "* .gz  binary\n"
            "* .jar binary\n"
            "* .rar binary\n"
            "* .tar binary\n"
            "* .zip binary\n"
            "\n"
            "## EXECUTABLES\n"
            "* .exe binary\n"
            "* .pyc binary\n";

    if (!AddTextInFile(pathToModule + "/" + ".gitattributes", _text))
        return false;

    auto CreateModuleDir = [&](std::string const& dirName)
    {
        Poco::File _file(pathToModule + "/" + dirName);

        try
        {
            _file.createDirectory();
        }
        catch (const Poco::Exception& e)
        {
            LOG_FATAL("> Failed create directory: %s", e.displayText().c_str());
        }
    };

    CreateModuleDir("conf");
    CreateModuleDir("src");

    LOG_INFO("> Base module files created sucessfull");

    return true;
}

void ModuleCreator::CreateModule(std::string const& moduleName, bool isWarheadModule /*= true*/)
{
    _IsWarheadModule = isWarheadModule;

    if (!CreateBaseArgs(moduleName))
        return;

    LOG_INFO("Creating %sCore module - %s", _IsWarheadModule ? "Warhead" : "Azeroth", moduleName.c_str());
    LOG_INFO("> Move base module files to new module - %s", correctModuleName.c_str());

    // #1. Create base files
    CreateBaseFiles();

    // #2. Create script_loader.cpp
    CreateScriptLoader(moduleName);

    // #3. Create class files
    CreateClassFiles(moduleName);

    // #4. Create SC file
    CreateSCFile(moduleName);

    // #5. Create config file
    CreateConfigFile(moduleName);

    // #6. Create CMakeLists.txt
    CreateCmakeFile(moduleName);
}

// Text transform
std::string ModuleCreator::GetHeadText(bool isSpecal /*= false*/)
{
    std::string text;

    if (!isSpecal)
    {

        text += "/*\n";
        text += "* This file is part of the WarheadCore Project. See AUTHORS file for Copyright information\n";
        text += "*\n";
        text += "* This program is free software; you can redistribute it and/or modify it\n";
        text += "* under the terms of the GNU General Public License as published by the\n";
        text += "* Free Software Foundation; either version 2 of the License, or (at your\n";
        text += "* option) any later version.\n";
        text += "*\n";
        text += "* This program is distributed in the hope that it will be useful, but WITHOUT\n";
        text += "* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\n";
        text += "* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for\n";
        text += "* more details.\n";
        text += "*\n";
        text += "* You should have received a copy of the GNU General Public License along\n";
        text += "* with this program. If not, see <http://www.gnu.org/licenses/>.\n";
        text += "*/";
    }
    else
    {
        text += "#\n";
        text += "# This file is part of the WarheadCore Project. See AUTHORS file for Copyright information\n";
        text += "#\n";
        text += "# This file is free software; as a special exception the author gives\n";
        text += "# unlimited permission to copy and/or distribute it, with or without\n";
        text += "# modifications, as long as this notice is preserved.\n";
        text += "#\n";
        text += "# This program is distributed in the hope that it will be useful, but\n";
        text += "# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the\n";
        text += "# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
        text += "#\n";
        text += "# User has manually chosen to ignore the git-tests, so throw them a warning.\n";
        text += "# This is done EACH compile so they can be alerted about the consequences.\n";
        text += "#";
    }

    return text;
}

std::string ModuleCreator::GetScriptsName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    return str;
}

std::string ModuleCreator::GetConfigFileName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".conf.dist";
}

std::string ModuleCreator::GetScriptLoaderFileName(std::string str)
{
    std::string text;
    std::transform(str.begin(), str.end(), std::back_inserter(text), tolower);
    return text +  "_loader." + (_IsWarheadModule ? "cpp" : "h");
}

std::string ModuleCreator::GetScriptCPPFileName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".cpp";
}

std::string ModuleCreator::GetSCCPPFileName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + "_SC.cpp";
}

std::string ModuleCreator::GetScriptHFileName(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '_'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    return str + ".h";
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
