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

#include "Config.h"
#include "Log.h"
#include "StringConvert.h"
#include "StringFormat.h"
#include "Util.h"
#include <mutex>
#include <fstream>
#include <unordered_map>

namespace
{
    std::string _filename;
    std::vector<std::string> _additonalFiles;
    std::unordered_map<std::string /*name*/, std::string /*value*/> _configOptions;
    std::mutex _configLock;

    void AddKey(std::string const& optionName, std::string const& optionKey, bool replace = true)
    {
        auto const& itr = _configOptions.find(optionName);
        if (itr != _configOptions.end())
        {
            if (!replace)
            {
                LOG_NOTICE("> Config: Option '%s' is exist! Option key - '%s'", optionName.c_str(), itr->second.c_str());
                return;
            }

            _configOptions.erase(optionName);
        }

        _configOptions.emplace(optionName, optionKey);

        LOG_DEBUG("> Config: Add '%s' - '%s'", optionName.c_str(), optionKey.c_str());
    }

    void ParseFile(std::string const& file)
    {
        std::ifstream in(file);

        if (in.fail())
            throw ConfigException(Warhead::StringFormat("Config::LoadFile: Failed open file '%s'", file.c_str()));

        uint32 count = 0;

        while (in.good())
        {
            std::string line;
            std::getline(in, line);

            if (line.empty())
                continue;

            line = Warhead::String::Trim(line, in.getloc());

            // comments
            if (line[0] == '#' || line[0] == '[')
                continue;

            size_t found = line.find_last_of('#');
            if (found != std::string::npos)
                line = line.substr(0, found);

            auto const equal_pos = line.find('=');

            if (equal_pos == std::string::npos || equal_pos == line.length())
                return;

            auto entry = Warhead::String::Trim(line.substr(0, equal_pos), in.getloc());
            auto value = Warhead::String::Trim(line.substr(equal_pos + 1), in.getloc());

            value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

            AddKey(entry, value);

            count++;
        }

        if (!count)
            throw ConfigException(Warhead::StringFormat("Config::LoadFile: Empty file '%s'", file.c_str()));
    }

    bool LoadFile(std::string const& file)
    {
        try
        {
            ParseFile(file);
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("> Config: %s", e.what());
        }

        return false;
    }
}

bool ConfigMgr::LoadInitial(std::string const& file)
{
    std::lock_guard<std::mutex> lock(_configLock);
    _configOptions.clear();
    return LoadFile(file);
}

bool ConfigMgr::LoadAdditionalFile(std::string file)
{
    std::lock_guard<std::mutex> lock(_configLock);
    return LoadFile(file);
}

ConfigMgr* ConfigMgr::instance()
{
    static ConfigMgr instance;
    return &instance;
}

bool ConfigMgr::Reload()
{
    if (!LoadAppConfigs())
        return false;

    return LoadModulesConfigs();
}

template<class T>
T ConfigMgr::GetValueDefault(std::string const& name, T const& def) const
{
    auto const& itr = _configOptions.find(name);
    if (itr == _configOptions.end())
    {
        LOG_WARN("> Config: Missing name %s in config, add \"%s = %s\"",
            name.c_str(), name.c_str(), Warhead::ToString(def).c_str());
        return def;
    }

    auto value = Warhead::StringTo<T>(itr->second);
    if (!value)
    {
        LOG_ERROR("> Config: Bad value defined for name '%s', going to use '%s' instead",
            name.c_str(), Warhead::ToString(def).c_str());
        return def;
    }

    return *value;
}

template<>
std::string ConfigMgr::GetValueDefault<std::string>(std::string const& name, std::string const& def) const
{
    auto const& itr = _configOptions.find(name);
    if (itr == _configOptions.end())
    {
        LOG_WARN("> Config: Missing name %s in config, add \"%s = %s\"",
            name.c_str(), name.c_str(), def.c_str());

        return def;
    }

    return itr->second;
}

template<class T>
T ConfigMgr::GetOption(std::string const& name, T const& def) const
{
    return GetValueDefault<T>(name, def);
}

template<>
WH_COMMON_API bool ConfigMgr::GetOption<bool>(std::string const& name, bool const& def) const
{
    std::string val = GetValueDefault(name, std::string(def ? "1" : "0"));

    auto boolVal = Warhead::StringTo<bool>(val);
    if (!boolVal)
    {
        LOG_ERROR("> Config: Bad value defined for name '%s', going to use '%s' instead",
            name.c_str(), def ? "true" : "false");
        return def;
    }

    return *boolVal;
}

std::string const& ConfigMgr::GetFilename()
{
    std::lock_guard<std::mutex> lock(_configLock);
    return _filename;
}

std::vector<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
    std::lock_guard<std::mutex> lock(_configLock);

    std::vector<std::string> keys;

    for (auto const& [optionName, key] : _configOptions)
        if (!optionName.compare(0, name.length(), name))
            keys.emplace_back(optionName);

    return keys;
}

std::string const ConfigMgr::GetConfigPath()
{
    std::lock_guard<std::mutex> lock(_configLock);

    return "configs/";
}

void ConfigMgr::Configure(std::string const& initFileName, std::string const& modulesConfigList /*= ""*/)
{
    _filename = initFileName;

    if (!modulesConfigList.empty())
        for (auto const& itr : Warhead::Tokenize(modulesConfigList, ',', false))
            _additonalFiles.emplace_back(std::string(itr));
}

bool ConfigMgr::LoadAppConfigs()
{
    // #1 - Load init config file .conf.dist
    if (!LoadInitial(_filename + ".dist"))
        return false;

    // #2 - Load .conf file
    if (!LoadAdditionalFile(_filename))
        return false;

    return true;
}

bool ConfigMgr::LoadModulesConfigs()
{
    if (_additonalFiles.empty())
        return false;

    // Start loading module configs
    std::vector<std::string /*config variant*/> moduleConfigFiles;
    std::string const& moduleConfigPath = GetConfigPath() + "modules/";
    bool isExistDefaultConfig = true;
    bool isExistDistConfig = true;

    for (auto const& distFileName : _additonalFiles)
    {
        std::string defaultFileName = distFileName;

        if (!defaultFileName.empty())
            defaultFileName.erase(defaultFileName.end() - 5, defaultFileName.end());

        // Load .conf.dist config
        if (!LoadAdditionalFile(moduleConfigPath + distFileName))
            isExistDistConfig = false;

        // Load .conf config
        if (!LoadAdditionalFile(moduleConfigPath + defaultFileName))
            isExistDefaultConfig = false;

        if (isExistDefaultConfig && isExistDistConfig)
            moduleConfigFiles.emplace_back(defaultFileName);
        else if (!isExistDefaultConfig && isExistDistConfig)
            moduleConfigFiles.emplace_back(distFileName);
    }

    // If module configs not exist - no load
    if (moduleConfigFiles.empty())
        return false;

    // Print modules configurations
    LOG_INFO("");
    LOG_INFO("Using modules configuration:");

    for (auto const& itr : moduleConfigFiles)
        LOG_INFO("> %s", itr.c_str());

    LOG_INFO("");

    return true;
}

#define TEMPLATE_CONFIG_OPTION(__typename) \
    template WH_COMMON_API __typename ConfigMgr::GetOption<__typename>(std::string const& name, __typename const& def) const;

TEMPLATE_CONFIG_OPTION(std::string)
TEMPLATE_CONFIG_OPTION(uint8)
TEMPLATE_CONFIG_OPTION(int8)
TEMPLATE_CONFIG_OPTION(uint16)
TEMPLATE_CONFIG_OPTION(int16)
TEMPLATE_CONFIG_OPTION(uint32)
TEMPLATE_CONFIG_OPTION(int32)
TEMPLATE_CONFIG_OPTION(uint64)
TEMPLATE_CONFIG_OPTION(int64)
TEMPLATE_CONFIG_OPTION(float)

#undef TEMPLATE_CONFIG_OPTION
