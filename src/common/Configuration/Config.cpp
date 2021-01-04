/*
 * Copyright
 */

#include "Config.h"
#include "Log.h"
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <memory>

using Poco::AutoPtr;
using Poco::Util::LayeredConfiguration;

AutoPtr<LayeredConfiguration> _configs;

ConfigMgr* ConfigMgr::instance()
{
    static ConfigMgr instance;
    return &instance;
}

void ConfigMgr::AddConfigFile(std::string const& file, bool enableDist /*= true*/)
{
    auto const itr = _configFiles.find(file);
    if (itr != _configFiles.end())
    {
        LOG_ERROR("Config file (%s) is exist", file.c_str());
        return;
    }

    _configFiles.insert(std::make_pair(file, enableDist));
}

bool ConfigMgr::Load()
{
    std::lock_guard<std::mutex> lock(_configLock);

    if (_configFiles.empty())
    {
        LOG_ERROR("Config files is empty");
        return false;
    }

    _configs = new LayeredConfiguration();

    for (auto const& itr : _configFiles)
    {
        try
        {
            _configs->add(new Poco::Util::PropertyFileConfiguration(itr.first));

            if (itr.second)
                _configs->add(new Poco::Util::PropertyFileConfiguration(itr.first + ".dist"));
        }
        catch (const Poco::Exception& e)
        {
            LOG_ERROR("Error at loading config - %s", e.displayText().c_str());
            return false;
        }
    }

    return true;
}

bool ConfigMgr::Reload()
{
    _configs.reset();

    if (Load())
        return true;

    return false;
}

std::string ConfigMgr::GetStringDefault(std::string const& name, const std::string& def) const
{
    auto key = def;

    try
    {
        key = _configs->getString(name);
    }
    catch (const Poco::Exception& e)
    {
        LOG_ERROR("Error at get config option - %s", e.displayText().c_str());
    }

    return key;
}

bool ConfigMgr::GetBoolDefault(std::string const& name, bool def) const
{
    auto key = def;

    try
    {
        key = _configs->getBool(name);
    }
    catch (const Poco::Exception& e)
    {
        LOG_ERROR("Error at get config option - %s", e.displayText().c_str());
    }

    return key;
}

int ConfigMgr::GetIntDefault(std::string const& name, int def) const
{
    auto key = def;

    try
    {
        key = _configs->getInt(name);
    }
    catch (const Poco::Exception& e)
    {
        LOG_ERROR("Error at get config option - %s", e.displayText().c_str());
    }

    return key;
}

float ConfigMgr::GetFloatDefault(std::string const& name, float def) const
{
    auto key = def;

    try
    {
        key = static_cast<float>(_configs->getDouble(name));
    }
    catch (const Poco::Exception& e)
    {
        LOG_ERROR("Error at get config option - %s", e.displayText().c_str());
    }

    return key;
}

std::set<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
    std::lock_guard<std::mutex> lock(_configLock);

    std::set<std::string> keys;
    Poco::Util::AbstractConfiguration::Keys range;

    _configs->keys(name, range);

    for (auto const& itr : range)
        keys.insert(itr);

    return keys;
}
