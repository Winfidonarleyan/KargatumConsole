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

#include "Merger.h"
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include "Util.h"
#include <Poco/RegularExpression.h>
#include <Poco/Exception.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>

namespace
{
    namespace fs = std::filesystem;

    uint32 filesFoundCount = 0;
    uint32 filesReplaceCount = 0;
    uint32 ReplaceLines = 0;
    fs::path _path;
    std::vector<fs::path> _localeFileStorage;
    std::vector<std::string> _supportExtensions;

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

    void Correct(fs::path const& path, std::string const& replaceFrom, std::string const& replaceTo)
    {
        std::string text = GetFileText(path.generic_string());

        uint8 count = Warhead::String::PatternReplace(text, replaceFrom, replaceTo);
        if (!count)
            return;

        filesReplaceCount++;
        ReplaceLines += count;

        LOG_INFO("%u. '%s'. Replace (%u)", filesReplaceCount, path.filename().generic_string().c_str(), count);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
            return;
        }

        file.write(text.c_str(), text.size());
        file.close();
    }

    void GetStats(uint32 startTimeMS)
    {
        LOG_INFO("");
        LOG_INFO("> Merger: ended correct for '%s'", _path.generic_string().c_str());
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

        // Clean before list add
        _localeFileStorage.clear();
        filesReplaceCount = 0;
        ReplaceLines = 0;

        FillFileList(_path);

        LOG_INFO("> Merger: Found '%u' files", filesFoundCount);
    }
}

Merger* Merger::instance()
{
    static Merger instance;
    return &instance;
}

void Merger::Init()
{
    SendPathInfo();

    GetListFiles({ ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    auto Replace = [](std::string const& replaceFrom, std::string const& replaceTo)
    {
        LOG_INFO("");
        LOG_INFO("> Merger: Start replace from '%s' to '%s'", replaceFrom.c_str(), replaceTo.c_str());

        filesReplaceCount = 0;
        ReplaceLines = 0;

        for (auto const& filePath : _localeFileStorage)
            Correct(filePath, replaceFrom, replaceTo);

        LOG_INFO("> Merger: Replace files (%u)", filesReplaceCount);
        LOG_INFO("> Merger: Replace lines (%u)", ReplaceLines);
    };

    LOG_INFO("> Start merge correct? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("> Merger: Start correct for '%s'", _path.generic_string().c_str());

    Replace("TrinityCore Project", "WarheadCore Project");
    Replace("TC_LOG", "LOG");
    Replace("TC_", "WH_");
    Replace("Trinity::", "Warhead::");
    Replace("namespace Trinity", "namespace Warhead");
    Replace("TrinityString", "WarheadString");

    GetStats(ms);
}

bool Merger::SetPath(std::string const& path)
{
    CleanPath();

    if (!fs::is_directory(path))
    {
        LOG_FATAL("> Merger: Path '%s' in not directory!", path.c_str());
        return false;
    }

    LOG_INFO("> Merger: Added path '%s'", path.c_str());

    _path = fs::path(path);

    return true;
}

std::string const Merger::GetPath()
{
    return _path.generic_string();
}

uint32 Merger::GetFoundFiles()
{
    return filesFoundCount;
}

void Merger::CleanPath()
{
    filesFoundCount = 0;
    _localeFileStorage.clear();
    _path.clear();
}

bool Merger::IsCorrectPath()
{
    return !_path.empty() && fs::is_directory(_path);
}

void Merger::SendPathInfo()
{
    CleanPath();

    auto GetPathFromConsole = [&](uint32 selectOption)
    {
        std::string whitespacePath;

        switch (selectOption)
        {
        case 1:
            whitespacePath = "F:\\Git\\Warhead\\src";
            break;
        case 2:
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
        LOG_INFO("1. ./src");
        LOG_INFO("2. ./cmake");
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
