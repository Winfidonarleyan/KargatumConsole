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
#include "StopWatch.h"
#include "ProgressBar.h"
#include "LaunchProcess.h"
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <iostream>
#include <fstream>
#include <map>
#include <boost/algorithm/string/join.hpp>

namespace fs = std::filesystem;

namespace
{
    constexpr auto PATH_TO_SRC = "src/";
    constexpr auto PATH_TO_MODULES = "modules/";

    // Common functions
    inline bool IsExitstText(std::string_view text, std::string_view textFind)
    {
        return text.find(textFind) != std::string::npos;
    }

    inline bool IsExitstText(std::string_view text, std::initializer_list<std::string_view> textFindList)
    {
        for (auto const& itr : textFindList)
            if (!IsExitstText(text, itr))
                return false;

        return true;
    }

    inline void GitCommit(std::string_view path, std::string_view cleanupName, std::string_view desc = {})
    {
        Warhead::Process::Git::CommitAllFiles(path, Warhead::StringFormat("chore(Cleanup): cleanup with WarheadCleanup. {}", cleanupName), desc);
    }

    inline void ShowStats(std::string_view cleanupName, StopWatch const& sw, ReplaceFilesStore const& stats, std::string_view path = {})
    {
        LOG_INFO("--");
        LOG_INFO("> Cleanup stats: {}", cleanupName);
        LOG_INFO("> Replaced files: {}", stats.size());
        LOG_INFO("> Elapsed time: {}", sw);
        LOG_INFO("--");

        if (stats.empty())
            return;

        std::size_t count{ 0 };

        for (auto const& [fileName, replacedLines] : stats)
            LOG_INFO("{}. File: {}. Lines: {}", ++count, fileName, replacedLines);

        LOG_INFO("--");

        if (path.empty() || !Warhead::Process::Git::IsRepository(path))
            return;

        LOG_INFO("> Found repository in dir '{}'", path);
        LOG_INFO("> Commit this changes? [yes (default) / no]");

        std::string select;
        std::getline(std::cin, select);

        if (!select.empty() && select.substr(0, 1) != "y")
            return;

        std::string desc = "-- Warhead cleanup stats:\n";
        count = 0;

        for (auto const& [fileName, replacedLines] : stats)
            desc.append(Warhead::StringFormat("{}. File: {}. Lines: {}\n", ++count, fileName, replacedLines));

        LOG_INFO("> Commit this changes. Description:\n{}", desc);
        GitCommit(path, cleanupName, desc);
    }

    // Sort includes
    std::unordered_map<std::string, std::string> _textStore;
}

Cleanup* Cleanup::instance()
{
    static Cleanup instance;
    return &instance;
}

void Cleanup::RemoveWhitespace()
{
    std::system("cls");

    SendPathInfo();

    if (IsCoreDir(_path))
    {
        LOG_INFO("> Found core dir. Check src and modules dirs only");

        fs::path pathtoSrc{ _path.generic_string() + PATH_TO_SRC };
        fs::path pathtoModules{ _path.generic_string() + PATH_TO_MODULES };

        FillFileList(pathtoSrc, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });
        FillFileList(pathtoModules, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });
    }
    else
        FillFileList(_path, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    if (_localeFileStorage.empty())
    {
        LOG_FATAL("Cleanup: Cannot continue, found 0 files.");
        return;
    }

    LOG_DEBUG("> Search complete. Found {} files", _localeFileStorage.size());

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    StopWatch sw;

    LOG_INFO("> Cleanup: Start cleanup (remove whitespace) for '{}'", _path.generic_string());

    ReplaceFilesStore stats;
    std::size_t size{ _localeFileStorage.size() };
    ProgressBar bar("", size);
    std::size_t count{ 0 };

    for (auto const& filePath : _localeFileStorage)
    {
        RemoveWhitespaceInFile(filePath, stats);
        bar.UpdatePostfixText(Warhead::StringFormat("{}/{}", ++count, size));
        bar.Update();
    }

    bar.Stop();

    ShowStats("Remove whitespace", sw, stats, _path.generic_string());
}

void Cleanup::ReplaceTabs()
{
    system("cls");

    SendPathInfo();

    if (IsCoreDir(_path))
    {
        LOG_INFO("> Found core dir. Check src and modules dirs only");

        fs::path pathtoSrc{ _path.generic_string() + PATH_TO_SRC };
        fs::path pathtoModules{ _path.generic_string() + PATH_TO_MODULES };

        FillFileList(pathtoSrc, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });
        FillFileList(pathtoModules, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });
    }
    else
        FillFileList(_path, { ".cpp", ".h", ".dist", ".conf", ".txt", ".cmake" });

    if (_localeFileStorage.empty())
    {
        LOG_FATAL("Cleanup: Cannot continue, found 0 files.");
        return;
    }

    LOG_DEBUG("> Search complete. Found {} files", _localeFileStorage.size());

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    StopWatch sw;

    LOG_INFO("> Cleanup: Start cleanup (replace tabs) for '{}'", _path.generic_string().c_str());

    ReplaceFilesStore stats;
    std::size_t size{ _localeFileStorage.size() };
    ProgressBar bar("", size);
    std::size_t count{ 0 };

    for (auto const& filePath : _localeFileStorage)
    {
        ReplaceTabstoWhitespaceInFile(filePath, stats);
        bar.UpdatePostfixText(Warhead::StringFormat("{}/{}", ++count, size));
        bar.Update();
    }

    bar.Stop();

    ShowStats("Replace tabs", sw, stats, _path.generic_string());
}

void Cleanup::SortIncludes()
{
    system("cls");

    SendPathInfo();

    if (IsCoreDir(_path))
    {
        LOG_INFO("> Found core dir. Check src and modules dirs only");

        fs::path pathtoSrc{ _path.generic_string() + PATH_TO_SRC };
        fs::path pathtoModules{ _path.generic_string() + PATH_TO_MODULES };

        FillFileList(pathtoSrc, { ".cpp", ".h" });
        FillFileList(pathtoModules, { ".cpp", ".h" });
    }
    else
        FillFileList(_path, { ".cpp", ".h" });

    if (_localeFileStorage.empty())
    {
        LOG_FATAL("Cleanup: Cannot continue, found 0 files.");
        return;
    }

    LOG_DEBUG("> Search complete. Found {} files", _localeFileStorage.size());

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    StopWatch sw;

    LOG_INFO("> Cleanup: Start cleanup (sort includes) for '{}'", _path.generic_string().c_str());

    ReplaceFilesStore stats;
    std::size_t size{ _localeFileStorage.size() };
    ProgressBar bar("", size);
    std::size_t count{ 0 };

    for (auto const& filePath : _localeFileStorage)
    {
        SortIncludesInFile(filePath, stats);
        bar.UpdatePostfixText(Warhead::StringFormat("{}/{}", ++count, size));
        bar.Update();
    }

    bar.Stop();

    ShowStats("Sort includes", sw, stats, _path.generic_string());
}

void Cleanup::CheckSameIncludes()
{
    system("cls");

    SendPathInfo();

    if (IsCoreDir(_path))
    {
        LOG_INFO("> Found core dir. Check src and modules dirs only");

        fs::path pathtoSrc{ _path.generic_string() + PATH_TO_SRC };
        fs::path pathtoModules{ _path.generic_string() + PATH_TO_MODULES };

        FillFileList(pathtoSrc, { ".cpp", ".h" });
        FillFileList(pathtoModules, { ".cpp", ".h" });
    }
    else
        FillFileList(_path, { ".cpp", ".h" });

    if (_localeFileStorage.empty())
    {
        LOG_FATAL("Cleanup: Cannot continue, found 0 files.");
        return;
    }

    LOG_DEBUG("> Search complete. Found {} files", _localeFileStorage.size());

    LOG_INFO("> Start cleanup? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    StopWatch sw;

    LOG_INFO("> Cleanup: Start cleanup (same includes) for '{}'", _path.generic_string().c_str());

    ReplaceFilesStore stats;
    std::size_t size{ _localeFileStorage.size() };
    ProgressBar bar("", size);
    std::size_t count{ 0 };

    for (auto const& filePath : _localeFileStorage)
    {
        CheckSameIncludes(filePath, stats);
        bar.UpdatePostfixText(Warhead::StringFormat("{}/{}", ++count, size));
        bar.Update();
    }

    bar.Stop();

    ShowStats("Same includes", sw, stats, _path.generic_string());
}

void Cleanup::CheckUsingIncludesCount()
{
    system("cls");

    SendPathInfo(true);

    FillFileList(_path, { ".cpp", ".h" });

    if (_localeFileStorage.empty())
    {
        LOG_FATAL("Cleanup: Cannot continue, found 0 files.");
        return;
    }

    LOG_DEBUG("> Search complete. Found {} files", _localeFileStorage.size());

    LOG_INFO("> Start job? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    LOG_INFO("> Cleanup: Start job (check includes count) for '{}'", _path.generic_string().c_str());

    CheckIncludesInFile();
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

void Cleanup::CleanPath()
{
    _localeFileStorage.clear();
    _path.clear();
}

bool Cleanup::IsCorrectPath()
{
    return !_path.empty() && fs::is_directory(_path);
}

void Cleanup::SendPathInfo(bool manually /*= false*/)
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

        if (!manually)
        {
            for (auto const& [index, _path] : _pathList)
            {
                LOG_INFO("{}. '{}'", index, _path.c_str());
            }

            LOG_INFO("# --");
            LOG_INFO("8. Enter path manually");
            LOG_INFO("> Select:");
        }
        else
        {
            LOG_INFO("# --");
            LOG_INFO("-- Enter path");
        }

        std::string selPathEnter;
        std::getline(std::cin, selPathEnter);

        system("cls");

        if (!SetPath(!manually ? GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter)) : selPathEnter))
            SendPathInfo(manually);
    }
    else
        LOG_INFO(">> Entered path '{}'", pathInfo);

    if (!IsCorrectPath())
        SendPathInfo(manually);
}

void Cleanup::LoadPathInfo()
{
    sConfigMgr->Configure("Cleanup.conf");

    if (!sConfigMgr->LoadAppConfigs())
    {
        LOG_FATAL("> Config not loaded!");
        return;
    }

    LOG_INFO(">> Loading path list...");

    _pathList.clear();

    std::string pathStr = sConfigMgr->GetOption<std::string>("Cleanup.PathList", "");
    if (pathStr.empty())
    {
        LOG_FATAL("Cleanup: Config option 'Cleanup.PathList' is empty!");
        return;
    }

    std::vector<std::string_view> tokens = Warhead::Tokenize(pathStr, ',', false);
    if (tokens.empty())
    {
        LOG_FATAL("Cleanup: Config option 'Cleanup.PathList' is incorrect!");
        return;
    }

    auto CorrectPath = [](std::string& path)
    {
        if (!path.empty() && (path.at(path.length() - 1) != '/') && (path.at(path.length() - 1) != '\\'))
            path.push_back('/');
    };

    std::size_t index{ 0 };

    for (auto const& path : tokens)
    {
        LOG_DEBUG("> Added path '{}'. Index {}", path, ++index);

        std::string toStr{ path };
        CorrectPath(toStr);
        _pathList.emplace(index, toStr);
    }

    LOG_INFO("> Loaded {} paths", _pathList.size());
    LOG_INFO("--");
}

// private fn
void Cleanup::FillFileList(std::filesystem::path const& path, std::initializer_list<std::string> extensionsList)
{
    if (!IsCorrectPath())
    {
        LOG_ERROR("Cleanup: Incorrect path at FillFileList. Path '{}'", path.generic_string());
        return;
    }

    LOG_DEBUG("> Search files in '{}'. Ext list: {}", path.generic_string(), boost::algorithm::join(extensionsList, " "));

    for (auto const& dirEntry : fs::recursive_directory_iterator(path))
    {
        auto const& path = dirEntry.path();

        auto const& find = std::find_if(std::begin(extensionsList), std::end(extensionsList), [path](std::string const& ext)
        {
            return path.has_extension() && path.extension() == ext;
        });

        if (find != std::end(extensionsList))
            _localeFileStorage.emplace_back(path);
    }
}

bool Cleanup::IsCoreDir(std::filesystem::path const& path)
{
    fs::path pathtoSrc{ path.generic_string() + PATH_TO_SRC };
    fs::path pathtoModules{ path.generic_string() + PATH_TO_MODULES };

    try
    {
        return fs::exists(pathtoSrc) && fs::exists(pathtoModules) && fs::is_directory(pathtoSrc) && fs::is_directory(pathtoSrc);
    }
    catch (fs::filesystem_error const& error)
    {
        LOG_ERROR("> Error at check path: {}", error.what());        
    }

    return false;
}

bool Cleanup::RemoveWhitespaceInFile(std::filesystem::path const& path, ReplaceFilesStore& stats)
{
    std::string text = Warhead::File::GetFileText(path.generic_string());

    uint32 replaceCount = Warhead::String::PatternReplace(text, "(^\\s+$)|( +$)", "");
    if (!replaceCount)
        return false;

    stats.emplace_back(path.filename().generic_string(), replaceCount);

    std::ofstream file(path);
    if (!file.is_open())
        return false;

    file.write(text.c_str(), text.size());
    file.close();
    return true;
}

void Cleanup::ReplaceTabstoWhitespaceInFile(std::filesystem::path const& path, ReplaceFilesStore& stats)
{
    std::string text = Warhead::File::GetFileText(path.generic_string());

    uint32 replaceCount = Warhead::String::PatternReplace(text, "(\\t)", "    ");
    if (!replaceCount)
        return;

    stats.emplace_back(path.filename().generic_string(), replaceCount);

    std::ofstream file(path);
    if (!file.is_open())
        return;

    file.write(text.c_str(), text.size());
    file.close();
}

void Cleanup::SortIncludesInFile(std::filesystem::path const& path, ReplaceFilesStore& stats)
{
    std::string fileText = Warhead::File::GetFileText(path.generic_string());
    std::string origText = fileText;
    std::vector<std::string> _includes;
    std::unordered_map<std::string, std::string> _toReplace;
    std::size_t replaceCount{ 0 };
    bool isFirstInclude = false;
    bool isFirst = false;
    bool isEnd = false;

    auto _SortIncludes = [&]()
    {
        std::string unsortIncludes;
        std::string sortIncludes;
        std::string firstInclude;

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
        replaceCount++;
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

            _includes.emplace_back(str);
        }
    }

    if (_toReplace.empty())
        return;

    for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
        fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

    if (fileText == origText)
        return;

    stats.emplace_back(path.filename().generic_string(), replaceCount);

    std::ofstream file(path);
    if (!file.is_open())
        return;

    file.write(fileText.c_str(), fileText.size());
    file.close();
}

void Cleanup::CheckSameIncludes(std::filesystem::path const& path, ReplaceFilesStore& stats)
{
    std::string fileText = Warhead::File::GetFileText(path.generic_string());
    std::string origText = fileText;
    std::vector<std::string> _includes;
    std::vector<std::string> _includesUniqueue;
    std::unordered_map<std::string, std::string> _toReplace;
    std::size_t replaceCount{ 0 };
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
            replaceCount++;
        }

        if (found != std::string::npos)
        {
            if (isFirst)
                isFirst = true;

            if (isEnd)
                isEnd = false;

            _includes.emplace_back(str);

            if (IsSameInclude(str))
                LOG_WARN("> Same include '{}' - '{}'", str, path.generic_string());
            else
                _includesUniqueue.emplace_back(str);
        }
    }

    if (_toReplace.empty())
        return;

    for (auto const& [unsortIncludes, sortIncludes] : _toReplace)
        fileText = Warhead::String::ReplaceInPlace(fileText, unsortIncludes, sortIncludes);

    if (fileText == origText)
        return;

    stats.emplace_back(path.filename().generic_string(), replaceCount);

    std::ofstream file(path.c_str());
    if (!file.is_open())
        return;

    file.write(fileText.c_str(), fileText.size());
    file.close();
}

void Cleanup::CheckIncludesInFile()
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
