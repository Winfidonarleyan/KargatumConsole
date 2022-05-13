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
#include "Config.h"
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include "Util.h"
#include "LaunchProcess.h"
#include "CryptoHash.h"
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <Poco/String.h>
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

    // Config
    std::unordered_map<uint8, std::string> _pathList;

    // Common functions
    bool IsExitstText(std::string_view text, std::string_view textFind)
    {
        return text.find(textFind) != std::string::npos;
    }

    bool IsExitstText(std::string_view text, std::initializer_list<std::string_view> textFindList)
    {
        for (auto const& itr : textFindList)
            if (!IsExitstText(text, itr))
                return false;

        return true;
    }

    void GitCommit()
    {
        Warhead::Process::Git::CommitAllFiles(_path.generic_string(), "\"chore(Core/Misc): code cleanup\"");
    }

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
        LOG_INFO("> Cleanup: ended cleanup for '{}'", _path.generic_string().c_str());
        LOG_INFO("# -- Found files ({})", filesFoundCount);
        LOG_INFO("# -- Replace files ({})", filesReplaceCount);

        if (ReplaceLines)
            LOG_INFO("# -- Replace lines ({})", ReplaceLines);

        LOG_INFO("# -- Used time '{}'", Warhead::Time::ToTimeString<Milliseconds>(GetMSTimeDiffToNow(startTimeMS), TimeOutput::Milliseconds).c_str());
        LOG_INFO("");
    }

    void GetListFiles(std::initializer_list<std::string> supportExtensions)
    {
        _supportExtensions = supportExtensions;

        FillFileList(_path);

        LOG_INFO("> Cleanup: Found '{}' files", filesFoundCount);
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
        LOG_INFO("{}. File ({}). Replace ({})", filesReplaceCount, path.filename().generic_string().c_str(), replaceCount);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
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
        LOG_INFO("{}. '{}'. Replace ({})", filesReplaceCount, path.filename().generic_string().c_str(), replaceCount);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
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

        auto _SortIncludes = [&]()
        {
            std::string unsortIncludes = "";
            std::string sortIncludes = "";
            std::string firstInclude = "";

            for (auto const& itr : _includes)
            {
                std::string nameFile = path.filename().generic_string();
                nameFile.erase(nameFile.end() - path.extension().generic_string().length(), nameFile.end());

                if (itr.find("#include \"" + nameFile + ".h") != std::string::npos)
                    firstInclude = itr;
            }

            for (auto const& itr : _includes)
                unsortIncludes += itr + "\n";

            std::sort(_includes.begin(), _includes.end());

            // Added first include
            if (!firstInclude.empty())
                sortIncludes += firstInclude + "\n";

            for (auto const& itr : _includes)
            {
                if (itr == firstInclude)
                    continue;

                sortIncludes += itr + "\n";
            }

            // If uncludes same - no need replace
            if (unsortIncludes == sortIncludes)
                return;

            _toReplace.emplace(unsortIncludes, sortIncludes);

            _includes.clear();
        };

        for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
        {
            auto found = str.find("#include");

            if (!isEnd && found == std::string::npos)
            {
                isEnd = true;

                if (_includes.empty() || _includes.size() == 1)
                    continue;

                _SortIncludes();
            }

            if (found != std::string::npos)
            {
                if (isFirst)
                    isFirst = true;

                if (isEnd)
                    isEnd = false;

                /*if (!isFirstInclude)
                {
                    isFirstInclude = true;

                    auto isDefaultInclude = str.find("#include \"") == std::string::npos;
                    if (isDefaultInclude || !enbaleCheckFirstInclude)
                        _includes.emplace_back(str);
                }
                else*/
                    _includes.emplace_back(str);
            }
        }

        for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
            fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("{}. '{}'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
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

                for (auto const& itr : _includes)
                    unsortIncludes += itr + "\n";

                // Add without same includes
                for (auto const& itr : _includesUniqueue)
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
                    LOG_WARN("> Same include '{}' - '{}'", std::string(str).c_str(), path.generic_string().c_str());
                else
                    _includesUniqueue.emplace_back(str);
            }
        }

        for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
            fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("{}. '{}'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path.c_str());
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
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

        LOG_INFO("{}. '{}'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
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

            LOG_INFO("{}. '{}' - {}", count, strInclude.c_str(), countInclude);
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

            LOG_INFO("{}. '{}'", count, option.c_str());
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
            LOG_FATAL("Failed open file \"{}\"!", filePath.c_str());
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
                LOG_FATAL("> Dubicate option ({})", find.c_str());
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
                        //LOG_INFO("> {} - {}", GetStringFnOld(std::string(resIndex2)).c_str(), GetStringFnNew(std::string(resMgr2)).c_str());
                    }
                }
            }
        };

        GetOptions();

        for (auto const& filePath : _localeFileStorage)
        {
            LOG_TRACE("> Replace file - ({})", filePath.generic_string().c_str());

            std::string fileText = Warhead::File::GetFileText(filePath.generic_string());
            std::string origText = fileText;

            for (auto const& [confIndex, confName] : _configOptions)
                fileText = Warhead::String::ReplaceInPlace(fileText, GetStringFnOld(confIndex), GetStringFnNew(confName));

            if (fileText == origText)
                continue;

            std::ofstream file(filePath);
            if (!file.is_open())
            {
                LOG_FATAL("Failed open file \"{}\"!", filePath.generic_string().c_str());
                continue;
            }

            file.write(fileText.c_str(), fileText.size());
            file.close();
        }
    }

    void ReplaceLoggingPattern(fs::path const& path)
    {
        std::string fileText = Warhead::File::GetFileText(path.generic_string());
        std::string origText = fileText;

        auto ReplacePattern = [](std::string& str, std::string_view from, std::string_view to, std::string_view::size_type start = 0)
        {
            std::string result;
            std::string::size_type pos = 0;

            result.append(str, 0, start);
            pos = str.find(from, start);

            if (pos != std::string::npos)
            {
                result.append(str, start, pos - start);
                result.append(to);
                start = pos + from.length();
            }
            else
                result.append(str, start, str.size() - start);

            str.swap(result);

            return str;
        };

        std::unordered_map<std::string, std::string> textLine;

        for (auto const& str : Warhead::Tokenize(fileText, '\n', true))
        {
            if (IsExitstText(str, { "LOG_", "%" }) ||
                IsExitstText(str, "StringFormat(") ||
                IsExitstText(str, "ASSERT") ||
                IsExitstText(str, "PSendSysMessage") ||
                IsExitstText(str, "PQuery") ||
                IsExitstText(str, "PExecute") ||
                IsExitstText(str, "PAppend") ||
                IsExitstText(str, "SendNotification"))
            {
                std::string text{ str };

                text = Warhead::String::ReplaceInPlace(text, "%s", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%u", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%hu", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%lu", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%llu", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%02u", "{:02}");
                text = Warhead::String::ReplaceInPlace(text, "%03u", "{:03}");
                text = Warhead::String::ReplaceInPlace(text, "%d", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%i", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%x", "{:x}");
                text = Warhead::String::ReplaceInPlace(text, "%X", "{:X}");
                text = Warhead::String::ReplaceInPlace(text, "%lx", "{:x}");
                text = Warhead::String::ReplaceInPlace(text, "%lX", "{:X}");
                text = Warhead::String::ReplaceInPlace(text, "%02X", "{:02X}");
                text = Warhead::String::ReplaceInPlace(text, "%08X", "{:08X}");
                text = Warhead::String::ReplaceInPlace(text, "%f", "{}");
                text = Warhead::String::ReplaceInPlace(text, "%.1f", "{0:.1f}");
                text = Warhead::String::ReplaceInPlace(text, "%.2f", "{0:.2f}");
                text = Warhead::String::ReplaceInPlace(text, "%.3f", "{0:.3f}");
                text = Warhead::String::ReplaceInPlace(text, "%.4f", "{0:.4f}");
                text = Warhead::String::ReplaceInPlace(text, "%.5f", "{0:.5f}");
                text = Warhead::String::ReplaceInPlace(text, "%3.1f", "{:3.1f}");
                text = Warhead::String::ReplaceInPlace(text, "%%", "%");
                text = Warhead::String::ReplaceInPlace(text, "GetGUID().ToString().c_str()", "GetGUID()");
                text = Warhead::String::ReplaceInPlace(text, ".c_str()", "");                

                textLine.emplace(str, text);
            }
        }

        if (textLine.empty())
            return;

        for (auto const& [_textOrig, _textReplaced] : textLine)
            fileText = Warhead::String::ReplaceInPlace(fileText, _textOrig, _textReplaced);

        if (fileText == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("{}. '{}'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
    }

    void ReplaceSQLVariable(fs::path const& path)
    {
        std::string text = Warhead::File::GetFileText(path.generic_string());
        std::string origText = text;

        auto ReplaceVariables = [&](std::string const& varName)
        {
            auto pattern = Warhead::StringFormat("( {0}\\([0-9]\\))|( {0}\\([0-9][0-9]\\))|( {0}\\([0-9][0-9][0-9]\\))", varName);
            auto upperVar = " " + Poco::toUpper(varName);

            auto replaceCount = Warhead::String::PatternReplace(text, pattern, upperVar);
            if (!replaceCount)
                return;

            ReplaceLines += replaceCount;            
        };

        ReplaceVariables("int");
        ReplaceVariables("tinyint");
        ReplaceVariables("smallint");
        ReplaceVariables("bigint");

        if (text == origText)
            return;

        filesReplaceCount++;

        LOG_INFO("{}. '{}'", filesReplaceCount, path.filename().generic_string().c_str());

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"{}\"!", path.generic_string().c_str());
            return;
        }

        file.write(text.c_str(), text.size());
        file.close();
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

    LOG_INFO("> Cleanup: Start cleanup (remove whitespace) for '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        RemoveWhitespaceInFile(filePath);

    GetStats(ms);

    if (filesReplaceCount)
    {
        LOG_INFO("> Commit this changes...");
        GitCommit();
    }
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

    LOG_INFO("> Cleanup: Start cleanup (replace tabs) for '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ReplaceTabstoWhitespaceInFile(filePath);

    GetStats(ms);
}

void Cleanup::SortIncludes()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    enbaleCheckFirstInclude = sConfigMgr->GetOption<bool>("Cleanup.CleanInclude.CheckFirst.Enable", true);

    LOG_INFO("# -- Sort Includes options: ");
    LOG_INFO("> Skip check first include - '{:s}'", enbaleCheckFirstInclude);
    LOG_INFO("# --");
    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (sort includes) for '{}'", _path.generic_string().c_str());

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

    LOG_INFO("> Cleanup: Start cleanup (same includes) for '{}'", _path.generic_string().c_str());

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

    LOG_INFO("> Cleanup: Start cleanup (remove extra logs define) for '{}'", _path.generic_string().c_str());

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

    LOG_INFO("> Cleanup: Start job (check includes count) for '{}'", _path.generic_string().c_str());

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

    LOG_INFO("> Cleanup: Start job (check config options '{}') for '{}'", configType.c_str(), _path.generic_string().c_str());

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

    LOG_INFO("> Cleanup: Start job (replace config options) for '{}'", _path.generic_string().c_str());

    ::ReplaceOldAPIConfigOptions("bool");
    ::ReplaceOldAPIConfigOptions("float");
    ::ReplaceOldAPIConfigOptions("int32");
    ::ReplaceOldAPIConfigOptions("uint32");
    ::ReplaceOldAPIConfigOptions("rate");
}

void Cleanup::ReplaceLoggingFormat()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    LOG_INFO("> Start replace? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start replace (logging format) for '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ::ReplaceLoggingPattern(filePath);

    GetStats(ms);
}

void Cleanup::RenameFiles()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".cpp", ".h" });

    LOG_INFO("> Start replace? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start replace (Manger to Mgr) for '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
    {
        auto fileName = filePath.filename().generic_string();
        auto found = fileName.find("Manager");

        if (found == std::string::npos)
            continue;

        auto& newFileName = fileName;
        Warhead::String::ReplaceInPlace(newFileName, "Manager", "Mgr");

        auto dirName = filePath.generic_string();
        dirName.erase(dirName.end() - fileName.length(), dirName.end());

        auto& oldPath = filePath;
        auto newPath = dirName + newFileName;

        if (oldPath == newPath)
            continue; // ?

        fs::rename(oldPath, newPath);
    }

    GetStats(ms);
}

void Cleanup::CheckSha1DBFiles()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".sql" });

    LOG_INFO("> Start check? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start check SHA1 hash '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    std::unordered_map<std::string, std::string> hashStore;
    std::string fileText = "--\nDELETE FROM `updates` WHERE `name` IN (";

    uint32 count = 1;

    for (auto const& filePath : _localeFileStorage)
    {
        std::string const hash = ByteArrayToHexStr(Warhead::Crypto::SHA1::GetDigestOf(Warhead::File::GetFileText(filePath.generic_string())));
        std::string const fileName = filePath.filename().generic_string();

        //LOG_INFO("{}. '{}' - '{}'", count, fileName, hash);

        hashStore.emplace(fileName, hash);
        count++;
    }

    for (auto const& [fileName, hash] : hashStore)
        fileText += "'" + fileName + "', ";

    // Delete last ( ,)
    if (!fileText.empty())
        fileText.erase(fileText.end() - 2, fileText.end());

    fileText += ");\nINSERT INTO `updates` (`name`, `hash`, `state`, `timestamp`, `speed`) VALUES\n";

    for (auto const& [fileName, hash] : hashStore)
        fileText += "('" + fileName + "', '" + hash + "', 'ARCHIVED', '2021-10-14 04:13:44', 1),\n";

    // Delete last (,)
    if (!fileText.empty())
        fileText.erase(fileText.end() - 2, fileText.end());

    fileText += ";";

    LOG_INFO("\n{}", fileText);

    std::ofstream file(_path.generic_string() + "/archive.sql");
    if (!file.is_open())
    {
        LOG_FATAL("Failed open file \"{}\"!", _path.generic_string());
        return;
    }

    file.write(fileText.c_str(), fileText.size());
    file.close();

    LOG_INFO("--");
}

void Cleanup::CorrectDBFiles()
{
    system("cls");

    SendPathInfo();

    GetListFiles({ ".sql" });

    LOG_INFO("> Start replace? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start replace (SQL variables) for '{}'", _path.generic_string().c_str());

    filesReplaceCount = 0;
    ReplaceLines = 0;

    for (auto const& filePath : _localeFileStorage)
        ::ReplaceSQLVariable(filePath);

    GetStats(ms);
}

// Path helpers
bool Cleanup::SetPath(std::string const& path)
{
    CleanPath();

    if (!fs::is_directory(path))
    {
        LOG_FATAL("> Cleanup: Path '{}' in not directory!", path.c_str());
        return false;
    }

    LOG_INFO("> Cleanup: Added path '{}'", path.c_str());

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
        std::string selectPath;

        if (selectOption == 8)
        {
            LOG_INFO("-- Enter path:");
            std::getline(std::cin, selectPath);
        }
        else
        {
            auto const& itr = _pathList.find(selectOption);
            if (itr != _pathList.end())
                selectPath = itr->second;
        }

        return selectPath;
    };

    std::string pathInfo = GetPath();
    if (pathInfo.empty())
    {
        LOG_FATAL(">> Path is empty! Please enter path.");

        for (auto const& [index, _path] : _pathList)
        {
            LOG_INFO("{}. '{}'", index, _path.c_str());
        }

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
        LOG_INFO(">> Entered path '{}'", pathInfo.c_str());

    if (!IsCorrectPath())
        SendPathInfo();
}

void Cleanup::LoadPathInfo()
{
    sConfigMgr->Configure("F:\\Git\\KargatumConsole\\src\\app\\Cleanup\\cleanup.conf");

    if (!sConfigMgr->LoadAppConfigs())
    {
        LOG_FATAL("> Config not loaded!");
        return;
    }

    LOG_INFO(">> Loading path list...");

    _pathList.clear();

    std::string pathStr = sConfigMgr->GetOption<std::string>("Cleanup.PathList", "");
    std::vector<std::string_view> tokens = Warhead::Tokenize(pathStr, ',', false);
    uint8 index = 1;

    for (auto const& itr : tokens)
    {
        LOG_DEBUG("> Added path '{}'. Index {}", std::string(itr).c_str(), index);
        _pathList.emplace(index, itr);
        index++;
    }

    LOG_INFO("> Loaded {} paths", static_cast<uint32>(_pathList.size()));
    LOG_INFO("--");
}


///

//#include <unordered_map>
//#include <unordered_set>
//#include "Util.h"
//#include <fstream>
//#include <optional>

//std::unordered_map<uint32, std::string> sqlColumns;
//std::unordered_map<uint32, std::string> sqlData;
//
//std::optional<std::string> GetColumnNameFromID(uint32 columnNumber)
//{
//    for (auto const& [_index, _columnName] : sqlColumns)
//        if (_index == columnNumber)
//            return _columnName;
//
//    return {};
//}
//
//bool IsColumnDisable(uint32 columnNumber)
//{
//    auto colName = GetColumnNameFromID(columnNumber);
//    if (!colName)
//        return false;
//
//    std::vector<std::string> disableColumns =
//    {
//        "`resistance1`", "`resistance2`", "`resistance3`", "`resistance4`", "`resistance5`", "`resistance6`",
//        "`spell1`", "`spell2`", "`spell3`", "`spell4`", "`spell5`", "`spell6`", "`spell7`", "`spell8`",
//        "`OfferRewardText`",
//        "`mindmg`", "`maxdmg`", "`attackpower`", "`minrangedmg`", "`maxrangedmg`", "`rangedattackpower`"
//    };
//
//    /*std::vector<std::string> disableColumns =
//    {
//        "`QuestInfoID`", "`QuestSortID`", "`Flags`", "`LogTitle`", "`LogDescription`", "`QuestDescription`", "`RequiredNpcOrGo1`", "`RequiredNpcOrGoCount1`"
//    };*/
//
//    for (auto const& itr : disableColumns)
//        if (*colName == itr)
//            return true;
//
//    return false;
//}
//
//std::optional<std::string> GetCorrectColumn(std::string const& columnName)
//{
//    std::unordered_map<std::string, std::string> incorrectColumns =
//    {
//        { "`Health_mod`", "`HealthModifier`"},
//        { "`Mana_mod`", "`ManaModifier`"},
//        { "`Armor_mod`", "`ArmorModifier`"},
//        { "`dmg_multiplier`", "`DamageModifier`"},
//    };
//
//    auto const& itr = incorrectColumns.find(columnName);
//    if (itr != incorrectColumns.end())
//        return itr->second;
//
//    return {};
//}
//
//void Print()
//{
//    LOG_INFO("");
//    LOG_INFO("");
//
//    auto const& fullText = Warhead::File::GetFileText("C:\\Users\\Dowla\\Desktop\\sql1.sql");
//    std::string newText;
//
//    for (auto const& text : Warhead::Tokenize(fullText, ';', false))
//    {
//        //LOG_INFO("'{}'", text);
//
//        auto startColumns = text.find_first_of("(");
//        auto endColumns = text.find_first_of(")");
//        auto resultStartText = text.substr(0, startColumns + 1);
//
//        sqlColumns.clear();
//        sqlData.clear();
//
//        if (startColumns != std::string::npos && endColumns != std::string::npos)
//        {
//            auto resultColumns = text.substr(startColumns + 1, endColumns - startColumns - 1);
//            auto resultStartText = text.substr(0, startColumns + 1);
//
//            uint32 count = 0;
//            for (auto const& itr : Warhead::Tokenize(resultColumns, ',', false))
//            {
//                std::string column = std::string(itr);
//                column = Warhead::String::Trim(column);
//                sqlColumns.emplace(count, column);
//                count++;
//            }
//
//            count = 0;
//
//            auto startData = text.find_last_of("(");
//            auto endData = text.find_last_of(")");
//
//            if (startData != std::string::npos && endData != std::string::npos)
//            {
//                auto resultData = text.substr(startData + 1, endData - startData - 1);
//
//                for (auto const& itr : Warhead::Tokenize(resultData, ',', false))
//                {
//                    std::string column = std::string(itr);
//                    column = Warhead::String::Trim(column);
//                    sqlData.emplace(count, column);
//                    count++;
//                }
//            }
//        }
//
//        std::string sqlQueryText = std::string(resultStartText);
//
//        if (sqlColumns.empty() || sqlData.empty())
//            continue;
//
//        for (auto const& [index, str] : sqlColumns)
//        {
//            if (IsColumnDisable(index))
//                continue;
//
//            std::string colName = str;
//
//            if (auto correctColumn = GetCorrectColumn(str))
//                colName = *correctColumn;
//
//            sqlQueryText.append(colName + ", ");
//        }
//
//        sqlQueryText.erase(sqlQueryText.size() - 2, 2);
//        sqlQueryText.append(") VALUES \n(");
//
//        for (auto const& [index, str] : sqlData)
//        {
//            if (IsColumnDisable(index))
//                continue;
//
//            sqlQueryText.append(str + ", ");
//        }
//
//        sqlQueryText.erase(sqlQueryText.size() - 2, 2);
//        sqlQueryText.append(");\n");
//
//        newText.append(sqlQueryText);
//    }
//
//    if (newText.empty())
//    {
//        LOG_FATAL("> newText empty!");
//        return;
//    }
//
//    auto filePath = "C:\\Users\\Dowla\\Desktop\\WHsql.sql";
//    std::ofstream file(filePath);
//    if (!file.is_open())
//    {
//        LOG_FATAL("Failed open file \"{}\"!", filePath);
//        return;
//    }
//
//    file.write(newText.c_str(), newText.size());
//    file.close();
//}
