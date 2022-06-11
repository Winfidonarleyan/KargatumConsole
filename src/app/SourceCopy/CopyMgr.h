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

#ifndef __SOURCE_COPY_H_
#define __SOURCE_COPY_H_

#include "Define.h"
#include <string>
#include <string_view>
#include <unordered_map>

namespace std::filesystem
{
    class path;
}

using SCPair = std::pair<std::string, std::string>;
using ListExtensions = std::vector<std::filesystem::path>;

class CopyMgr
{
    CopyMgr() = default;
    ~CopyMgr() = default;
    CopyMgr(CopyMgr const&) = delete;
    CopyMgr(CopyMgr&&) = delete;
    CopyMgr& operator=(CopyMgr const&) = delete;
    CopyMgr& operator=(CopyMgr&&) = delete;

public:
    static CopyMgr* instance();
    bool Load();

    void SendBaseInfo();

private:
    void CreatePathFromConfig(std::string_view configScName);

    bool IsExist(std::string const& scName);
    SCPair const* GetSC(std::string const& scName);
    SCPair const* GetSC(uint8 index);

    auto GetIndexStore() { return &_indexStore; }
    std::string_view GetSCNameFromIndex(uint8 index);

    void CopyFiles(std::string_view type, std::string_view optionalPath, ListExtensions extensions = {});

    bool FillFileList(std::filesystem::path const& path, ListExtensions* extensions = nullptr);

    //
    bool SetSC(uint8 index);
    bool IsCorrectSC();
    void SendSCInfo();

    std::unordered_map<std::string, SCPair> _store;
    std::unordered_map<uint8, std::string> _indexStore;

    std::string _selectedSC;
    std::vector<std::filesystem::path> _localeFileStorage;
};

#define sCopyMgr CopyMgr::instance()

#endif
