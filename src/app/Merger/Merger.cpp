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
#include "Config.h"
#include "StringConvert.h"
#include "Timer.h"
#include "Util.h"
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
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

    std::unordered_map<std::string/*from*/, std::string/*to*/> _replaceStore;

    // Config
    std::unordered_map<uint8, std::string> _pathList;

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

    void CorrectText(fs::path const& path)
    {
        std::string text = Warhead::File::GetFileText(path.generic_string());
        std::string origText{ text };
        ReplaceLines = 0;

        for (auto const& [_from, _to] : _replaceStore)
        {
            text = Warhead::String::Replace(text, _from, _to);
            if (text == origText)
                continue;

            ReplaceLines++;
        }

        if (origText == text)
            return;

        LOG_INFO("merger", "{}. '{}'. Replace ({})", filesReplaceCount, path.filename().generic_string().c_str(), ReplaceLines);

        std::ofstream file(path);
        if (!file.is_open())
        {
            LOG_FATAL("merger", "Failed open file \"{}\"!", path.generic_string().c_str());
            return;
        }

        filesReplaceCount++;

        file.write(text.c_str(), text.size());
        file.close();
    }

    void GetStats(uint32 startTimeMS)
    {
        LOG_INFO("merger", "");
        LOG_INFO("merger", "> Merger: ended correct for '{}'", _path.generic_string().c_str());
        LOG_INFO("merger", "# -- Found files ({})", filesFoundCount);
        LOG_INFO("merger", "# -- Replace files ({})", filesReplaceCount);

        if (ReplaceLines)
            LOG_INFO("merger", "# -- Replace lines ({})", ReplaceLines);

        //LOG_INFO("merger", "# -- Used time '{}'", Warhead::Time::ToTimeString(GetMSTimeDiffToNow(startTimeMS)));
        LOG_INFO("merger", "");
    }

    void GetListFiles(std::initializer_list<std::string> supportExtensions)
    {
        _supportExtensions = supportExtensions;

        // Clean before list add
        _localeFileStorage.clear();
        filesReplaceCount = 0;
        ReplaceLines = 0;

        FillFileList(_path);

        LOG_INFO("merger", "> Merger: Found '{}' files", filesFoundCount);
    }
}

Merger* Merger::instance()
{
    static Merger instance;
    return &instance;
}

void Merger::Init()
{
    LoadPathInfo();
    SendPathInfo();

    GetListFiles({ ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    auto AddReplaceStore = [&](std::string_view from, std::string_view to)
    {
        auto const& itr = _replaceStore.find(std::string(from));
        if (itr != _replaceStore.end())
            return;

        _replaceStore.emplace(from, to);
    };

    auto Replace = [&]()
    {
        LOG_INFO("merger", "");
        LOG_INFO("merger", "> Merger: Start replace...");

        filesReplaceCount = 0;
        ReplaceLines = 0;

        for (auto const& filePath : _localeFileStorage)
            CorrectText(filePath);

        LOG_INFO("merger", "> Merger: Replace files ({})", filesReplaceCount);
        LOG_INFO("merger", "> Merger: Replace lines ({})", ReplaceLines);
    };

    LOG_INFO("merger", "> Start merge correct? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    uint32 ms = getMSTime();

    LOG_INFO("merger", "> Merger: Start correct for '{}'", _path.generic_string().c_str());

    // License
    AddReplaceStore("AzerothCore", "WarheadCore");

    //// Logging
    //AddReplaceStore("FMT_LOG", "LOG");

    //// Macros
    //Replace("AC_COMMON_API", "WH_COMMON_API");
    //Replace("AC_GAME_API", "WH_GAME_API");

    //// Namespace
    AddReplaceStore("Acore::", "Warhead::");
    AddReplaceStore("namespace Acore", "namespace Warhead");

    //// Strings
    AddReplaceStore("AcoreString", "WarheadString");

    // DB PS
    AddReplaceStore("setNull(",         "SetData(");
    AddReplaceStore("setBool(",         "SetData(");
    AddReplaceStore("setUInt8(",        "SetData(");
    AddReplaceStore("setInt8(",         "SetData(");
    AddReplaceStore("setUInt16(",       "SetData(");
    AddReplaceStore("setInt16(",        "SetData(");
    AddReplaceStore("setUInt32(",       "SetData(");
    AddReplaceStore("setInt32(",        "SetData(");
    AddReplaceStore("setUInt64(",       "SetData(");
    AddReplaceStore("setInt64(",        "SetData(");
    AddReplaceStore("setFloat(",        "SetData(");
    AddReplaceStore("setDouble(",       "SetData(");
    AddReplaceStore("setString(",       "SetData(");
    AddReplaceStore("setStringView(",   "SetData(");
    AddReplaceStore("setBinary(",       "SetData(");

    // DB Fields
    AddReplaceStore("GetBool(",         "Get<bool>(");
    AddReplaceStore("GetUInt8(",        "Get<uint8>(");
    AddReplaceStore("GetInt8(",         "Get<int8>(");
    AddReplaceStore("GetUInt16(",       "Get<uint16>(");
    AddReplaceStore("GetInt16(",        "Get<int16>(");
    AddReplaceStore("GetUInt32(",       "Get<uint32>(");
    AddReplaceStore("GetInt32(",        "Get<int32>(");
    AddReplaceStore("GetUInt64(",       "Get<uint64>(");
    AddReplaceStore("GetInt64(",        "Get<int64>(");
    AddReplaceStore("GetFloat(",        "Get<float>(");
    AddReplaceStore("GetDouble(",       "Get<double>(");
    AddReplaceStore("GetString(",       "Get<std::string>(");
    AddReplaceStore("GetStringView(",   "Get<std::string_view>(");
    AddReplaceStore("GetBinary(",       "Get<Binary>(");

    // Start work
    Replace();

    GetStats(ms);
}

bool Merger::SetPath(std::string const& path)
{
    CleanPath();

    if (!fs::is_directory(path))
    {
        LOG_FATAL("merger", "> Cleanup: Path '{}' in not directory!", path.c_str());
        return false;
    }

    LOG_INFO("merger", "> Cleanup: Added path '{}'", path.c_str());

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
        std::string selectPath;

        if (selectOption == 8)
        {
            LOG_INFO("merger", "-- Enter path:");
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
        LOG_FATAL("merger", ">> Path is empty! Please enter path.");

        for (auto const& [index, _path] : _pathList)
        {
            LOG_INFO("merger", "{}. '{}'", index, _path.c_str());
        }

        LOG_INFO("merger", "# --");
        LOG_INFO("merger", "8. Enter path manually");
        LOG_INFO("merger", "> Select:");

        std::string selPathEnter;
        std::getline(std::cin, selPathEnter);

        system("cls");

        if (!SetPath(GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter))))
            SendPathInfo();
    }
    else
        LOG_INFO("merger", ">> Entered path '{}'", pathInfo.c_str());

    if (!IsCorrectPath())
        SendPathInfo();
}

void Merger::LoadPathInfo()
{
    sConfigMgr->Configure("F:\\Git\\KargatumConsole\\src\\app\\Cleanup\\cleanup.conf");

    if (!sConfigMgr->LoadAppConfigs())
    {
        LOG_FATAL("merger", "> Config not loaded!");
        return;
    }

    LOG_INFO("merger", ">> Loading path list...");

    _pathList.clear();

    std::string pathStr = sConfigMgr->GetOption<std::string>("Cleanup.PathList", "");
    std::vector<std::string_view> tokens = Warhead::Tokenize(pathStr, ',', false);
    uint8 index = 1;

    for (auto const& itr : tokens)
    {
        LOG_DEBUG("merger", "> Added path '{}'. Index {}", std::string(itr).c_str(), index);
        _pathList.emplace(index, itr);
        index++;
    }

    LOG_INFO("merger", "> Loaded {} paths", _pathList.size());
    LOG_INFO("merger", "--");
}
