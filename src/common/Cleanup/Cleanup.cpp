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
    uint32 ReplaceWhitespace = 0;
    constexpr uint32 MAX_DEPTH = 10;
    fs::path _path;
    std::vector<fs::path> _localeFileStorage;
    std::vector<std::string> _supportExtensions = { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" };

    std::string GetFileText(std::string const& path)
    {
        std::ifstream in(path);
        if (!in.is_open())
        {
            LOG_FATAL("> Failed to open file (%s)", path.c_str());
            return "";
        }

        auto text = [&in]
        {
            std::ostringstream ss;
            ss << in.rdbuf();
            return ss.str();
        }();

        in.close();
        return std::move(text);
    }

    void FillFileListRecursively(fs::path const& path, uint32 const depth)
    {
        auto isNormalExtension = [](fs::path const& path)
        {
            for (auto const& _ext : _supportExtensions)
                if (_ext == path)
                    return true;

            return false;
        };

        for (auto& dirEntry : fs::directory_iterator(path))
        {
            auto const& path = dirEntry.path();

            if (fs::is_directory(path) && depth < MAX_DEPTH)
                FillFileListRecursively(path, depth + 1);
            else if (isNormalExtension(path.extension()))
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
            ReplaceWhitespace += count;
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

        //file << text;
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
            ReplaceWhitespace += count;
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

        //file << text;
        file.write(text.c_str(), text.size());
        file.close();
    }

    void GetStats(uint32 startTimeMS)
    {
        LOG_INFO("");
        LOG_INFO("> Cleanup: ended cleanup for '%s'", _path.generic_string().c_str());
        LOG_INFO("# -- Found files (%u)", filesFoundCount);
        LOG_INFO("# -- Replace files (%u)", filesReplaceCount);
        LOG_INFO("# -- Replace lines (%u)", ReplaceWhitespace);
        LOG_INFO("# -- Used time '%u' ms", GetMSTimeDiffToNow(startTimeMS));
        LOG_INFO("");
    }
}

Cleanup* Cleanup::instance()
{
    static Cleanup instance;
    return &instance;
}

void Cleanup::CheckWhitespace()
{
    system("cls");

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (whitespace) for '%s'", _path.generic_string().c_str());
    LOG_INFO("> Cleanup: Vector size '%u'", static_cast<uint32>(_localeFileStorage.size()));

    filesReplaceCount = 0;
    ReplaceWhitespace = 0;

    for (auto const& filePath : _localeFileStorage)
        RemoveWhitespaceInFile(filePath);

    GetStats(ms);
}

void Cleanup::CheckTabs()
{
    system("cls");

    uint32 ms = getMSTime();

    LOG_INFO("> Cleanup: Start cleanup (tabs) for '%s'", _path.generic_string().c_str());
    LOG_INFO("> Cleanup: Vector size '%u'", static_cast<uint32>(_localeFileStorage.size()));

    filesReplaceCount = 0;
    ReplaceWhitespace = 0;

    for (auto const& filePath : _localeFileStorage)
        ReplaceTabstoWhitespaceInFile(filePath);

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

    FillFileListRecursively(path, 1);

    LOG_INFO("> Cleanup: Found '%u' files", filesFoundCount);

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
