/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "Define.h"
#include <stdexcept>
#include <string_view>
#include <vector>

class WH_COMMON_API ConfigMgr
{
    ConfigMgr() = default;
    ~ConfigMgr() = default;
    ConfigMgr(ConfigMgr const&) = delete;
    ConfigMgr(ConfigMgr&&) = delete;
    ConfigMgr& operator=(ConfigMgr const&) = delete;
    ConfigMgr& operator=(ConfigMgr&&) = delete;

public:
    static bool LoadAppConfigs();
    static void Configure(std::string_view initFileName);

    static ConfigMgr* instance();

    static std::string const GetFilename();
    static std::string const GetConfigPath();
    static std::vector<std::string> GetKeysByString(std::string const& name);

    template<class T>
    T GetOption(std::string const& name, T const& def, bool showLogs = true) const;

private:
    static bool LoadInitial(std::string_view file);
    static bool LoadAdditionalFile(std::string_view file);

    template<class T>
    T GetValueDefault(std::string const& name, T const& def, bool showLogs = true) const;
};

class WH_COMMON_API ConfigException : public std::length_error
{
public:
    explicit ConfigException(std::string const& message) : std::length_error(message) { }
};

#define sConfigMgr ConfigMgr::instance()

#endif