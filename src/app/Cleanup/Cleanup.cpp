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

#include "Cleanup.h"
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include <Poco/RegularExpression.h>
#include <Poco/Exception.h>
#include "Poco/Mutex.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

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

    void ReplaceTabstoWhitespaceInFile(fs::path const& path)
    {
        std::string text = GetFileText(path.generic_string());

        try
        {
            Poco::RegularExpression re("(\\t)", Poco::RegularExpression::RE_MULTILINE);
            uint8 count = re.subst(text, " ", Poco::RegularExpression::RE_GLOBAL);

            if (!count)
                return;

            filesReplaceCount++;
            ReplaceLines += count;
            LOG_INFO("%u. File (%s). Replace (%u)", filesReplaceCount, path.filename().generic_string().c_str(), count);
        }
        catch (const Poco::Exception& e)
        {
            LOG_FATAL("%s", e.displayText());
        }

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
        std::string text = GetFileText(path.generic_string());

        try
        {
            Poco::RegularExpression re("(^\\s+$)|( +$)", Poco::RegularExpression::RE_MULTILINE);
            uint8 count = re.subst(text, "", Poco::RegularExpression::RE_GLOBAL);

            if (!count)
                return;

            filesReplaceCount++;
            ReplaceLines += count;
            LOG_INFO("%u. '%s'. Replace (%u)", filesReplaceCount, path.filename().generic_string().c_str(), count);
        }
        catch (const Poco::Exception& e)
        {
            LOG_FATAL("%s", e.displayText());
        }

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
        std::string fileText = GetFileText(path.generic_string());
        std::string origText = fileText;
        std::vector<std::string> _includes;
        std::unordered_map<std::string, std::string> _toReplace;

        bool isFirstInclude = false;
        bool isFirst = false;
        bool isEnd = false;

        for (auto const& itr : Warhead::Tokenize(fileText, '\n', true))
        {
            std::string str{ itr };

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

        std::ofstream file(path.c_str());
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(fileText.c_str(), fileText.size());
        file.close();
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
                SendPathInfo();
                break;
        }

        return whitespacePath;
    };

    std::string pathInfo = GetPath();
    if (pathInfo.empty())
    {
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

        system("cls");

        if (!SetPath(GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter))))
            SendPathInfo();
    }
    else
        LOG_INFO(">> Entered path '%s'", pathInfo.c_str());

    if (!IsCorrectPath())
        SendPathInfo();
}
