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

#include "Define.h"
#include "GitRevision.h"
#include "Log.h"
#include "StringConvert.h"
#include "Config.h"
#include "LaunchProcess.h"
#include <iostream>
#include <filesystem>
#include <unordered_map>

namespace
{
    namespace fs = std::filesystem;

    fs::path _path;
    fs::path _configPath;

    std::string upstreamName;
}

bool IsCorrectConfig()
{
    upstreamName = sConfigMgr->GetOption<std::string>("PrTest.UpstreamName", "off");
    return Warhead::Process::Git::IsCorrectUpstream(_path.generic_string(), upstreamName);
}

// Path helpers
bool IsCorrectPath(std::string_view path)
{
    return !path.empty() && fs::is_directory(path);
}

void LoadPathInfo()
{
    _configPath.clear();

    sConfigMgr->Configure("F:\\Git\\KargatumConsole\\src\\app\\PrTest\\PrTest.conf");

    if (!sConfigMgr->LoadAppConfigs())
    {
        LOG_FATAL("pr", "> Config not loaded!");
        return;
    }

    LOG_INFO("pr", ">> Loading path to repo...");

    _configPath = fs::path(sConfigMgr->GetOption<std::string>("PrTest.Path", ""));

    if (!IsCorrectPath(_configPath.generic_string()))
    {
        LOG_ERROR("pr", "> Path ({}) is incorrect!", _configPath.generic_string());
        _configPath.clear();
        return;
    }

    LOG_DEBUG("pr", "> Added path '{}'", _configPath.generic_string());
    LOG_INFO("pr", "--");
}

bool SetPath(std::string const& path)
{
    _path.clear();

    if (!IsCorrectPath(path))
    {
        LOG_FATAL("pr", "> PR test: Path '{}' is not directory!", path);
        return false;
    }

    LOG_INFO("pr", "> PR test: Added path '{}'", path);

    _path = fs::path(path);

    return true;
}

void SendPathInfo()
{
    auto GetPathFromConsole = [&](uint32 selectOption)
    {
        std::string selectPath;

        if (selectOption == 1)
        {
            LOG_INFO("pr", "-- Enter path:");
            std::getline(std::cin, selectPath);
        }
        else if (selectOption == 2)
            selectPath = _configPath.generic_string();

        return selectPath;
    };

    std::string pathInfo = _path.generic_string();
    if (pathInfo.empty())
    {
        LOG_FATAL("pr", ">> Path is empty! Please select or enter path to AC repo.");
        LOG_INFO("pr", "1. Enter path manually");

        if (!_configPath.empty())
            LOG_INFO("pr", "2. Set {}", _configPath.generic_string());
        
        LOG_INFO("pr", "> Select:");

        std::string selPathEnter;
        std::getline(std::cin, selPathEnter);

        system("cls");

        if (!SetPath(GetPathFromConsole(*Warhead::StringTo<uint32>(selPathEnter))))
            SendPathInfo();
    }
    else
        LOG_INFO("pr", ">> Entered path '{}'", pathInfo.c_str());

    if (!IsCorrectPath(_path.generic_string()))
        SendPathInfo();
}

void TestPR()
{
    system("cls");
    SendPathInfo();

    if (!IsCorrectConfig())
        return;

    LOG_INFO("pr", "> Start chechout pr? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    system("cls");

    // Start
    LOG_INFO("pr", "> Enter PR ID:");
    std::getline(std::cin, select);

    std::string branchName = "pr-" + select;

    LOG_INFO("pr", "-- Options:");
    LOG_INFO("pr", "> 1. Upstream - ({})", upstreamName);
    LOG_INFO("pr", "> 2. Path to AC repo - ({})", _path.generic_string());
    LOG_INFO("pr", "> 3. PR number - ({})", select);
    LOG_INFO("pr", "--");

    LOG_INFO("pr", "> Start Checkout AC PR");

    if (Warhead::Process::Git::IsExistBranch(_path.generic_string(), branchName))
    {
        LOG_ERROR("pr", "> PR {} and branch name ({}) is exist in local repo!", select, branchName);
        return;
    }

    Warhead::Process::Git::Checkout(_path.generic_string(), "master");
    Warhead::Process::Git::CreateBranch(_path.generic_string(), branchName);
    Warhead::Process::Git::Checkout(_path.generic_string(), branchName);
    Warhead::Process::Git::CheckoutPR(_path.generic_string(), upstreamName, select);

    LOG_INFO("pr", "Go next? [yes (default) / no]");
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        exit(0);
}

void Select()
{
    LOG_INFO("pr", "# -- Select option:");
    LOG_INFO("pr", "# 1. Test AzerothCore PR");
    LOG_INFO("pr", "# --");
    LOG_INFO("pr", "# 99. Exit");
    LOG_INFO("pr", "# --");
    LOG_INFO("pr", "> Select:");

    std::string selOptionCleanup;
    std::getline(std::cin, selOptionCleanup);
    uint32 cleanupMethod = *Warhead::StringTo<uint32>(selOptionCleanup);

    switch (cleanupMethod)
    {
        case 1:
            TestPR();
            break;
        // Exit console
        case 99:
            exit(0);
            break;
        default:
            Select();
            break;
    }
}

int main()
{
    LOG_INFO("pr", "> {}", GitRevision::GetFullVersion());
    LOG_INFO("pr", "--");

    LoadPathInfo();

    while (true)
    {
        Select();
    }
   
    return 0;
}
