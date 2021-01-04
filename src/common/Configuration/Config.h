/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "Define.h"
#include <list>
#include <unordered_map>
#include <mutex>
#include <set>

class WH_COMMON_API ConfigMgr
{
    ConfigMgr() = default;
    ConfigMgr(ConfigMgr const&) = delete;
    ConfigMgr& operator=(ConfigMgr const&) = delete;
    ~ConfigMgr() = default;

public:
    static ConfigMgr* instance();

    bool Load();
    bool Reload();
    void AddConfigFile(std::string const& file, bool enableDist = true);

    std::string GetStringDefault(std::string const& name, const std::string& def) const;
    bool GetBoolDefault(std::string const& name, bool def) const;
    int GetIntDefault(std::string const& name, int def) const;
    float GetFloatDefault(std::string const& name, float def) const;

    std::set<std::string> GetKeysByString(std::string const& name);

private:
    std::unordered_map<std::string, bool> _configFiles;
    std::mutex _configLock;
};

#define sConfigMgr ConfigMgr::instance()

#endif
