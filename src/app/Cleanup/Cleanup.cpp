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
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include "Util.h"
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <tuple>

namespace
{
    namespace fs = std::filesystem;

    uint32 filesFoundCount = 0;
    uint32 filesReplaceCount = 0;
    uint32 ReplaceLines = 0;
    fs::path _path;
    std::vector<fs::path> _localeFileStorage;
    std::vector<std::string> _supportExtensions;

    // Sort includes
    std::unordered_map<std::string, std::string> _textStore;
    bool enbaleCheckFirstInclude = true;

    bool IsNormalExtension(fs::path const& path)
    {
        for (auto const& _ext : _supportExtensions)
            if (_ext == path)
                return true;

        return false;
    }

    void FillFileList(fs::path const& path)
    {
        for (auto& dirEntry : fs::directory_iterator(path))
        {
            auto const& path = dirEntry.path();

            if (fs::is_directory(path))
                FillFileList(path);
            else if (IsNormalExtension(path.extension()))
            {
                _localeFileStorage.emplace_back(path.generic_string());
                filesFoundCount++;
            }
        }
    }

    void GetStats(uint32 startTimeMS)
    {
        LOG_INFO("");
        LOG_INFO("> Cleanup: ended cleanup for '%s'", _path.generic_string().c_str());
        LOG_INFO("# -- Found files (%u)", filesFoundCount);
        LOG_INFO("# -- Replace files (%u)", filesReplaceCount);

        if (ReplaceLines)
            LOG_INFO("# -- Replace lines (%u)", ReplaceLines);

        LOG_INFO("# -- Used time '%s'", Warhead::Time::ToTimeString<Milliseconds>(GetMSTimeDiffToNow(startTimeMS), TimeOutput::Milliseconds).c_str());
        LOG_INFO("");
    }

    void GetListFiles(std::initializer_list<std::string> supportExtensions)
    {
        _supportExtensions = supportExtensions;

        FillFileList(_path);

        LOG_INFO("> Cleanup: Found '%u' files", filesFoundCount);
    }

    // General functions
    void ReplaceTabstoWhitespaceInFile(fs::path const& path)
    {
        std::string text = Warhead::File::GetFileText(path.generic_string());

        uint32 replaceCount = Warhead::String::PatternReplace(text, "(\\t)", " ");
        if (!replaceCount)
            return;

        filesReplaceCount++;
        ReplaceLines += replaceCount;
        LOG_INFO("%u. File (%s). Replace (%u)", filesReplaceCount, path.filename().generic_string().c_str(), replaceCount);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(text.c_str(), text.size());
        file.close();
    }

    void RemoveWhitespaceInFile(fs::path const& path)
    {
        std::string text = Warhead::File::GetFileText(path.generic_string());

        uint32 replaceCount = Warhead::String::PatternReplace(text, "(^\\s+$)|( +$)", "");
        if (!replaceCount)
            return;

        filesReplaceCount++;
        ReplaceLines += replaceCount;
        LOG_INFO("%u. '%s'. Replace (%u)", filesReplaceCount, path.filename().generic_string().c_str(), replaceCount);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(text.c_str(), text.size());
        file.close();
    }

    void SortIncludesInFile(fs::path const& path)
    {
        std::string fileText = Warhead::File::GetFileText(path.generic_string());
        std::string origText = fileText;
        std::vector<std::string> _includes;
        std::unordered_map<std::string, std::string> _toReplace;

        bool isFirstInclude = false;
        bool isFirst = false;
        bool isEnd = false;

        for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
        {
            //std::string str{ itr };

            auto found = str.find("#include");

            if (!isEnd && found == std::string::npos)
            {
                isEnd = true;

                if (_includes.empty() || _includes.size() == 1)
                    continue;

                std::string unsortIncludes = "";
                std::string sortIncludes = "";

                for (auto const itr : _includes)
                    unsortIncludes += itr + "\n";

                std::sort(_includes.begin(), _includes.end());

                for (auto const itr : _includes)
                    sortIncludes += itr + "\n";

                // If uncludes same - no need replace
                if (unsortIncludes == sortIncludes)
                    continue;

                _toReplace.emplace(unsortIncludes, sortIncludes);

                _includes.clear();
            }

            if (found != std::string::npos)
            {
                if (isFirst)
                    isFirst = true;

                if (isEnd)
                    isEnd = false;

                if (!isFirstInclude)
                {
                    isFirstInclude = true;

                    auto isDefaultInclude = str.find("#include \"");
                    if (isDefaultInclude == std::string::npos || !enbaleCheckFirstInclude)
                        _includes.emplace_back(str);
                }
                else
                    _includes.emplace_back(str);
            }
        }

        for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
            fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("%u. '%s'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path.c_str());
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
    }

    void CheckSameIncludes(fs::path const& path)
    {
        std::string fileText = Warhead::File::GetFileText(path.generic_string());
        std::string origText = fileText;
        std::vector<std::string> _includes;
        std::vector<std::string> _includesUniqueue;
        std::unordered_map<std::string, std::string> _toReplace;

        bool isFirstInclude = false;
        bool isFirst = false;
        bool isEnd = false;

        auto IsSameInclude = [&](std::string_view includeName)
        {
            for (auto const& itr : _includesUniqueue)
                if (itr == includeName)
                    return true;

            return false;
        };

        for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
        {
            auto found = str.find("#include");

            if (!isEnd && found == std::string::npos)
            {
                isEnd = true;

                if (_includes.empty() || _includes.size() == 1)
                    continue;

                std::string unsortIncludes = "";
                std::string sortIncludes = "";

                for (auto const itr : _includes)
                    unsortIncludes += itr + "\n";

                // Add without same includes
                for (auto const itr : _includesUniqueue)
                    sortIncludes += itr + "\n";

                // If uncludes same - no need replace
                if (unsortIncludes == sortIncludes)
                    continue;

                _toReplace.emplace(unsortIncludes, sortIncludes);

                _includes.clear();
                _includesUniqueue.clear();
            }

            if (found != std::string::npos)
            {
                if (isFirst)
                    isFirst = true;

                if (isEnd)
                    isEnd = false;

                _includes.emplace_back(str);

                if (IsSameInclude(str))
                    LOG_WARN("> Same include '%s' - '%s'", std::string(str).c_str(), path.generic_string().c_str());
                else
                    _includesUniqueue.emplace_back(str);
            }
        }

        for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
            fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("%u. '%s'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path.c_str());
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
    }

    void CleanExtraDefines(fs::path const& path)
    {
        std::string fileText = Warhead::File::GetFileText(path.generic_string());

        std::string origText = fileText;
        std::map<uint32 /*line number*/, std::string /*line text*/> lineTexts;
        std::vector<uint32> _toDeleteLines;
        uint32 lineNumber = 1;

        // Insert default text
        for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
        {
            lineTexts.emplace(lineNumber, str);
            lineNumber++;
        }

        bool isFoundDefine = false;
        bool isFoundEndIf = false;
        lineNumber = 1;

        for (auto& [stringLine, stringText] : lineTexts)
        {
            auto foundDefine = stringText.find("ENABLE_EXTRA_LOGS");
            auto foundEndIf = stringText.find("#endif");

            if (!isFoundDefine && foundDefine != std::string::npos)
            {
                isFoundDefine = true;
                isFoundEndIf = false;
                _toDeleteLines.emplace_back(lineNumber);
                ReplaceLines++;
            }
            else if (isFoundDefine && !isFoundEndIf && foundEndIf != std::string::npos)
            {
                isFoundEndIf = true;
                isFoundDefine = false;
                _toDeleteLines.emplace_back(lineNumber);
                ReplaceLines++;
            }

            lineNumber++;
        }

        fileText.clear();

        for (auto itr : _toDeleteLines)
            lineTexts.erase(itr);

        for (auto const& [stringLine, stringText] : lineTexts)
            fileText += stringText + "\n";

        // Replace whitespace
        Warhead::String::PatternReplace(fileText, "(^\\s+$)|( +$)", "");

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("%u. '%s'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
    }

    void CheckIncludesInFile()
    {
        std::unordered_map<std::string, uint32> _includes;
        std::multimap<uint32, std::string, std::greater<uint32>> sortIncludes;

        for (auto const& filePath : _localeFileStorage)
        {
            std::string fileText = Warhead::File::GetFileText(filePath.generic_string());

            bool isFirstInclude = false;
            bool isFirst = false;
            bool isEnd = false;

            for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
            {
                auto found = str.find("#include");

                if (found != std::string::npos)
                {
                    auto itr = _includes.find(std::string(str));
                    if (itr == _includes.end())
                        _includes.emplace(str, 1);
                    else
                        itr->second++;
                }
            }
        }

        uint32 count = 0;

        for (auto const& [strInclude, countInclude] : _includes)
            sortIncludes.emplace(countInclude, strInclude);

        for (auto const& [countInclude, strInclude] : sortIncludes)
        {
            if (countInclude < 3)
                continue;

            count++;

            LOG_INFO("%u. '%s' - %u", count, strInclude.c_str(), countInclude);
        }
    }

    void CheckConfigOptions(std::string const& configType)
    {
        std::vector<std::string> _configOptions;

        auto IsExistOption = [&](std::string find)
        {
            for (auto const& itr : _configOptions)
                if (find == itr)
                    return true;

            return false;
        };

        std::string stringFind = "sConfigMgr->GetOption<" + configType + ">(\"";

        for (auto const& filePath : _localeFileStorage)
        {
            std::string fileText = Warhead::File::GetFileText(filePath.generic_string());

            for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
            {
                auto found = str.find(stringFind);
                if (found != std::string::npos)
                {
                    auto res1 = str.substr(found + stringFind.length());
                    auto found1 = res1.find_first_of('"');

                    if (found1 != std::string::npos)
                    {
                        auto res2 = res1.substr(0, found1);

                        if (!IsExistOption(std::string(res2)))
                            _configOptions.emplace_back(res2);
                    }
                }
            }
        }

        std::sort(_configOptions.begin(), _configOptions.end());

        uint32 count = 0;
        std::string fileText = "AddOption({ ";

        for (auto const& option : _configOptions)
        {
            count++;

            LOG_INFO("%u. '%s'", count, option.c_str());
            fileText.append("\"" + option + "\"," + "\n");
        }

        // Delete last (,\n)
        if (!fileText.empty())
            fileText.erase(fileText.end() - 2, fileText.end());

        fileText.append(" });");

        std::string filePath = "F:\\Git\\WarheadBand\\src\\server\\Configs_" + configType + ".txt";

        if (configType == "std::string")
            filePath = "F:\\Git\\WarheadBand\\src\\server\\Configs_std_string.txt";

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", filePath.c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
    }

    void ReplaceOldAPIConfigOptions(std::string const& configType)
    {
        std::unordered_map<std::string/*index*/, std::string /*option*/> _configOptions;

        auto IsExistOption = [&](std::string find)
        {
            auto const& itr = _configOptions.find(find);
            if (itr != _configOptions.end())
            {
                LOG_FATAL("> Dubicate option (%s)", find.c_str());
                return true;
            }

            return false;
        };

        auto GetStringFnOld = [&](std::string const& confIndex) -> std::string
        {
            if (configType == "bool")
                return "sWorld->getBoolConfig(" + confIndex + ")";
            else if (configType == "int32" || configType == "uint32")
                return "sWorld->getIntConfig(" + confIndex + ")";
            else if (configType == "float")
                return "sWorld->getFloatConfig(" + confIndex + ")";
            else if (configType == "rate")
                return "sWorld->getRate(" + confIndex + ")";

            LOG_FATAL("GetStringFnOld");
            return "";
        };

        auto GetStringFnNew = [&](std::string const& confName) -> std::string
        {
            if (configType == "bool")
                return "CONF_GET_BOOL(\"" + confName + "\")";
            else if (configType == "int32")
                return "CONF_GET_INT(\"" + confName + "\")";
            else if (configType == "uint32")
                return "CONF_GET_UINT(\"" + confName + "\")";
            else if (configType == "float" || configType == "rate")
                return "CONF_GET_FLOAT(\"" + confName + "\")";

            LOG_FATAL("GetStringFnNew");
            return "";
        };

        auto GetOptions = [&]()
        {
            std::string stringFindIndex;
            std::string stringFindMgr = "sConfigMgr->GetOption<" + configType + ">(\"";

            if (configType == "bool")
                stringFindIndex = "m_bool_configs[";
            else if (configType == "int32" || configType == "uint32")
                stringFindIndex = "m_int_configs[";
            else if (configType == "float")
                stringFindIndex = "m_float_configs[";
            else if (configType == "rate")
            {
                stringFindIndex = "rate_values[";
                stringFindMgr = "sConfigMgr->GetOption<float>(\"";
            }

            std::string _fileText = Warhead::File::GetFileText("F:\\Git\\WarheadBand\\src\\server\\game\\World\\World.cpp");

            for (auto const& str : Warhead::Tokenize(_fileText, '\n', true))
            {
                auto foundMgr = str.find(stringFindMgr);
                auto foundIndex = str.find(stringFindIndex);

                if (foundMgr != std::string::npos && foundIndex != std::string::npos)
                {
                    auto resMgr1 = str.substr(foundMgr + stringFindMgr.length());
                    auto foundMgr1 = resMgr1.find_first_of('"');

                    auto resIndex1 = str.substr(foundIndex + stringFindIndex.length());
                    auto foundIndex1 = resIndex1.find_first_of(']');

                    if (foundMgr1 != std::string::npos && foundIndex1 != std::string::npos)
                    {
                        auto resMgr2 = resMgr1.substr(0, foundMgr1);
                        auto resIndex2 = resIndex1.substr(0, foundIndex1);

                        _configOptions.emplace(resIndex2, resMgr2);
                        //LOG_INFO("> %s - %s", GetStringFnOld(std::string(resIndex2)).c_str(), GetStringFnNew(std::string(resMgr2)).c_str());
                    }
                }
            }
        };

        GetOptions();

        for (auto const& filePath : _localeFileStorage)
        {
            LOG_TRACE("> Replace file - (%s)", filePath.generic_string().c_str());

            std::string fileText = Warhead::File::GetFileText(filePath.generic_string());
            std::string origText = fileText;

            for (auto const& [confIndex, confName] : _configOptions)
                fileText = Warhead::String::ReplaceInPlace(fileText, GetStringFnOld(confIndex), GetStringFnNew(confName));

            if (fileText == origText)
                continue;

            std::ofstream file(filePath);
            if (!file.is_open())
            {
                LOG_FATAL("Failed open file \"%s\"!", filePath.generic_string().c_str());
                continue;
            }

            file.write(fileText.c_str(), fileText.size());
            file.close();
        }
    }
}

Cleanup* Cleanup::instance()
{
    static Cleanup instance;
    return &instance;
}

void Cleanup::RemoveWhitespace()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (remove whitespace) for '%s'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        RemoveWhitespaceInFile(filePath);

    GetStats(ms);
}

void Cleanup::ReplaceTabs()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (replace tabs) for '%s'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ReplaceTabstoWhitespaceInFile(filePath);

    GetStats(ms);
}

void Cleanup::SortIncludes(bool needCheckFirstInclude /*= false*/)
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    enbaleCheckFirstInclude = needCheckFirstInclude;

    LOG_INFO("# -- Sort Includes options: ");
    LOG_INFO("1. Skip check first include - '%s'", enbaleCheckFirstInclude ? "true" : "false");
    LOG_INFO("# --");
    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (sort includes) for '%s'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        SortIncludesInFile(filePath);

    GetStats(ms);
}

void Cleanup::CheckSameIncludes()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (same includes) for '%s'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ::CheckSameIncludes(filePath);

    GetStats(ms);
}

void Cleanup::CheckExtraLogs()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h"});

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (remove extra logs define) for '%s'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ::CleanExtraDefines(filePath);

    GetStats(ms);
}

void Cleanup::CheckUsingIncludesCount()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h"});

    LOG_INFO("> Start job? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start job (check includes count) for '%s'", _path.generic_string().c_str());

    ::CheckIncludesInFile();
}

void Cleanup::CheckConfigOptions(std::string const& configType)
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    LOG_INFO("> Start job? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start job (check config options '%s') for '%s'", configType.c_str(), _path.generic_string().c_str());

    ::CheckConfigOptions(configType);
}

void Cleanup::ReplaceConfigOptions()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    LOG_INFO("> Start job? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start job (replace config options) for '%s'", _path.generic_string().c_str());

    ::ReplaceOldAPIConfigOptions("bool");
    ::ReplaceOldAPIConfigOptions("float");
    ::ReplaceOldAPIConfigOptions("int32");
    ::ReplaceOldAPIConfigOptions("uint32");
    ::ReplaceOldAPIConfigOptions("rate");
}

// Path helpers
bool Cleanup::SetPath(std::string const& path)
{
    CleanPath();

    if (!fs::is_directory(path))
    {
        LOG_FATAL("> Cleanup: Path '%s' in not directory!", path.c_str());
        return false;
    }

    LOG_INFO("> Cleanup: Added path '%s'", path.c_str());

    _path = fs::path(path);

    return true;
}

std::string const Cleanup::GetPath()
{
    return _path.generic_string();
}

uint32 Cleanup::GetFoundFiles()
{
    return filesFoundCount;
}

void Cleanup::CleanPath()
{
    filesFoundCount = 0;
    _localeFileStorage.clear();
    _path.clear();
}

bool Cleanup::IsCorrectPath()
{
    return !_path.empty() && fs::is_directory(_path);
}

void Cleanup::SendPathInfo()
{
    CleanPath();

    auto GetPathFromConsole = [&](uint32 selectOption)
    {
        std::string whitespacePath;

        switch (selectOption)
        {
            case 1:
                whitespacePath = "F:\\Git\\WarheadBand\\src";
                break;
            case 2:
                whitespacePath = "F:\\Git\\azerothcore-wotlk\\src";
                break;
            case 8:
                LOG_INFO("-- Enter path:");
                std::getline(std::cin, whitespacePath);
                break;
            default:
                SendPathInfo();
                break;
        }

        return whitespacePath;
    };

    std::string pathInfo = GetPath();
    if (pathInfo.empty())
    {
        LOG_FATAL(">> Path is empty! Please enter path.");
        LOG_INFO("1. ./WarheadBand/src");
        LOG_INFO("2. ./azerothcore-wotlk/src");
        LOG_INFO("# --");
        LOG_INFO("8. Enter path manually");
        LOG_INFO("> Select:");

        std::string selPathEnter;
        std::getline(std::cin, selPathEnter);

        system("cls");

        if (!SetPath(GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter))))
            SendPathInfo();
    }
    else
        LOG_INFO(">> Entered path '%s'", pathInfo.c_str());

    if (!IsCorrectPath())
        SendPathInfo();
}
