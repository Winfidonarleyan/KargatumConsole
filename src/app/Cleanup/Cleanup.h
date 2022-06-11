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

#ifndef _CLEANUP_H_
#define _CLEANUP_H_

#include "Common.h"
#include "Duration.h"
#include <filesystem>
#include <unordered_map>
#include <vector>

using ReplaceFiles = std::pair<std::string, std::size_t>;
using ReplaceFilesStore = std::vector<ReplaceFiles>;

class WH_COMMON_API Cleanup
{
public:
    static Cleanup* instance();

    void RemoveWhitespace();
    void ReplaceTabs();
    void SortIncludes();
    void CheckSameIncludes();
    void CheckUsingIncludesCount();
    
    void LoadPathInfo();

private:
    bool SetPath(std::string const& path);
    void CleanPath();
    std::string const GetPath();
    bool IsCorrectPath();
    void SendPathInfo(bool manually = false);

    void FillFileList(std::filesystem::path const& path, std::initializer_list<std::string> extensionsList);
    bool IsCoreDir(std::filesystem::path const& path);

    // General functions
    bool RemoveWhitespaceInFile(std::filesystem::path const& path, ReplaceFilesStore& stats);
    void ReplaceTabstoWhitespaceInFile(std::filesystem::path const& path, ReplaceFilesStore& stats);
    void SortIncludesInFile(std::filesystem::path const& path, ReplaceFilesStore& stats);
    void CheckSameIncludes(std::filesystem::path const& path, ReplaceFilesStore& stats);
    void CheckIncludesInFile();

    // Config
    std::unordered_map<std::size_t, std::string> _pathList;

    uint32 _filesReplaceCount = 0;
    uint32 _replaceLines = 0;
    std::filesystem::path _path;
    std::vector<std::filesystem::path> _localeFileStorage;
    std::vector<std::string> _supportExtensions;
};

#define sClean Cleanup::instance()

#endif // _CLEANUP_H_
