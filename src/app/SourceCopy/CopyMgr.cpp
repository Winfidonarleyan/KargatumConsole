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

#include "CopyMgr.h"
#include "Config.h"
#include "Log.h"
#include "Util.h"
#include "StringConvert.h"
#include "ProgressBar.h"
#include <filesystem>
#include <iostream>
#include <optional>

namespace fs = std::filesystem;

namespace
{
    // Prefix's
    constexpr auto PREFIX_PATH = "SC.Path.";
    constexpr auto PREFIX_SC = "SC.List";
    constexpr auto PREFIX_PATH_LENGTH = 8;

    constexpr auto PATH_TO_BIN = "build/bin/RelWithDebInfo/";
}

CopyMgr* CopyMgr::instance()
{
    static CopyMgr instance;
    return &instance;
}

bool CopyMgr::Load()
{
    sConfigMgr->Configure("SourceCopy.conf");

    if (!sConfigMgr->LoadAppConfigs())
    {
        LOG_FATAL("copy", "> Config not loaded!");
        return false;
    }

    auto const& keys = sConfigMgr->GetKeysByString(PREFIX_SC);
    if (!keys.size())
    {
        LOG_ERROR("copy", "CopyMgr::Load: Not found sc, change config file!");
        return false;
    }

    for (auto const& scName : keys)
        CreatePathFromConfig(scName);

    if (_store.empty())
    {
        LOG_ERROR("copy", "> SC list is empty!");
        return false;
    }

    uint8 index{ 0 };

    for (auto const& itr : _store)
    {
        _indexStore.emplace(++index, itr.first);
        LOG_DEBUG("copy", "> Added SC {} - '{}'", index, itr.first);
    }

    LOG_INFO("copy", "> Loaded {} source copy paths", _indexStore.size());

    return true;
}

std::string_view CopyMgr::GetSCNameFromIndex(uint8 index)
{
    auto const& itr = _indexStore.find(index);
    if (itr == _indexStore.end())
        return {};

    return std::string_view{ itr->second };
}

bool CopyMgr::IsExist(std::string const& scName)
{
    return _store.find(scName) != _store.end();
}

SCPair const* CopyMgr::GetSC(std::string const& scName)
{
    if (scName.empty())
        return nullptr;

    auto const& itr = _store.find(scName);
    if (itr == _store.end())
        return nullptr;

    return &itr->second;
}

SCPair const* CopyMgr::GetSC(uint8 index)
{
    return GetSC(std::string{ GetSCNameFromIndex(index) });
}

void CopyMgr::CreatePathFromConfig(std::string_view configScName)
{
    if (configScName.empty())
        return;

    if (IsExist(std::string(configScName)))
    {
        LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: SC for ({}) exist!", configScName);
        return;
    }

    std::string const options = sConfigMgr->GetOption<std::string>(std::string(configScName), "");
    auto scName = configScName.substr(PREFIX_PATH_LENGTH);

    if (options.empty())
    {
        LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: Missing config option ({})", configScName);
        return;
    }

    auto const& tokens = Warhead::Tokenize(options, ' ', true);
    if (!tokens.size() || tokens.size() != 2)
    {
        LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: Bad config options for SC ({})", scName);
        return;
    }

    auto CheckPath = [scName](fs::path const& path)
    {
        if (path.empty())
        {
            LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: Empty path for SC ({})", path.generic_string(), scName);
            return false;
        }

        try
        {
            if (!fs::exists(path))
            {
                LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: Non exist path '{}' for SC ({})", path.generic_string(), scName);
                return false;
            }

            if (!fs::is_directory(path))
            {
                LOG_ERROR("copy", "CopyMgr::CreatePathFromConfig: Path is not directory '{}' for SC ({})", path.generic_string(), scName);
                return false;
            }
        }
        catch (fs::filesystem_error const& error)
        {
            LOG_ERROR("copy", "> Error at check path: {}", error.what());
            return false;
        }

        return true;
    };

    auto CorrectPath = [](std::string& path)
    {
        if (!path.empty() && (path.at(path.length() - 1) != '/') && (path.at(path.length() - 1) != '\\'))
            path.push_back('/');
    };

    std::string configSourcePath = sConfigMgr->GetOption<std::string>(PREFIX_PATH + std::string(tokens.at(0)), "");
    std::string configBinaryPath = sConfigMgr->GetOption<std::string>(PREFIX_PATH + std::string(tokens.at(1)), "");

    CorrectPath(configSourcePath);
    CorrectPath(configBinaryPath);

    fs::path sourcePath{ configSourcePath };
    fs::path binaryPath{ configBinaryPath };

    if (!CheckPath(sourcePath) || !CheckPath(binaryPath))
        return;

    _store.emplace(std::string(scName), SCPair{ sourcePath.generic_string(), binaryPath.generic_string() });
}

bool CopyMgr::SetSC(uint8 index)
{
    _selectedSC.clear();

    auto scName = GetSCNameFromIndex(index);

    if (scName.empty())
    {
        LOG_FATAL("copy", "> SC: Index '{}' is incorrect!", index);
        return false;
    }

    LOG_INFO("copy", "> Selected SC '{}'", scName);

    _selectedSC = scName;

    return true;
}

bool CopyMgr::IsCorrectSC()
{
    if (_selectedSC.empty())
        return false;

    auto const& sc = GetSC(_selectedSC);
    if (!sc)
        return false;

    fs::path sourceDir{ sc->first };
    fs::path binaryDir{ sc->second };

    return fs::is_directory(sourceDir) && fs::is_directory(binaryDir);
}

void CopyMgr::SendSCInfo()
{
    if (_selectedSC.empty())
    {
        LOG_FATAL("copy", ">> Please select SC name.");

        for (auto const& [index, scName] : _indexStore)
        {
            LOG_INFO("copy", "{}. '{}'", index, scName);
        }

        LOG_INFO("copy", "# --");
        LOG_INFO("copy", "> Select:");

        std::string selIndex;
        std::getline(std::cin, selIndex);

        system("cls");

        if (!SetSC(*Warhead::StringTo<uint8>(selIndex)))
            SendSCInfo();
    }
    else
        LOG_INFO("copy", ">> Selected SC '{}'", _selectedSC);

    if (!IsCorrectSC())
        SendSCInfo();
}

void CopyMgr::SendBaseInfo()
{
    LOG_INFO("copy", ">> Welcome");
    LOG_INFO("copy", "--");

    SendSCInfo();

    if (_selectedSC.empty())
        ABORT("SC is empty!");

    LOG_INFO("copy", "> Start copy? [yes (default) / no]");

    std::string select;
    std::getline(std::cin, select);

    if (!select.empty() && select.substr(0, 1) != "y")
        return;

    CopyFiles("binary", {}, { ".exe", ".dll", ".pdb" });
    CopyFiles("default config", "configs/", { ".dist" });
    CopyFiles("modules config", "configs/modules/", { ".dist" });

    // Dll's scripts
    CopyScripts();

    LOG_WARN("copy", "> All steps done.");

    std::getline(std::cin, select);
}

bool CopyMgr::FillFileList(fs::path const& path, ListExtensions* extensions /*= nullptr*/)
{
    _localeFileStorage.clear();

    try
    {
        if (!fs::exists(path))
        {
            LOG_ERROR("copy", "> Empty dir '{}'", path.generic_string());
            return false;
        }

        if (!fs::is_directory(path))
        {
            LOG_ERROR("copy", "> Incorrect dir '{}'", path.generic_string());
            return false;
        }
    }
    catch (fs::filesystem_error const& error)
    {
        LOG_ERROR("> Error at check path: {}", error.what());
        return false;
    }

    for (auto const& dirEntry : fs::directory_iterator(path))
    {
        if (dirEntry.is_directory())
            continue;

        if (extensions && !extensions->empty() &&
            std::find(extensions->begin(), extensions->end(), dirEntry.path().extension()) == std::end(*extensions))
            continue;

        _localeFileStorage.emplace_back(dirEntry.path());
    }

    return true;
}

void CopyMgr::CopyFiles(std::string_view type, std::string_view optionalPath, ListExtensions extensions /*= {}*/)
{
    LOG_INFO("copy", "> Prepare to copy {} files", type);

    auto const& sc = GetSC(_selectedSC);
    if (!sc)
    {
        LOG_ERROR("copy", "> Not found paths for SC '{}'", _selectedSC);
        return;
    }

    fs::path sourceDir{ sc->first };
    sourceDir += PATH_TO_BIN;

    if (!optionalPath.empty())
        sourceDir += optionalPath;

    fs::path binaryDir{ sc->second };

    if (!optionalPath.empty())
        binaryDir += optionalPath;

    if (!FillFileList(sourceDir, &extensions))
    {
        LOG_ERROR("copy", "> Skip this step");
        return;
    }

    if (_localeFileStorage.empty())
        return;

    LOG_DEBUG("copy", "> Start copy {} files", type);
    LOG_DEBUG("copy", "--");
    LOG_DEBUG("copy", "> Found {} files", _localeFileStorage.size());
    LOG_DEBUG("copy", "> From: {}. To: {}", sourceDir.generic_string(), binaryDir.generic_string());
    LOG_DEBUG("copy", "--");

    try
    {
        if (!fs::exists(binaryDir))
        {
            LOG_ERROR("copy", "> Dir '{}' don't exist", binaryDir.generic_string());
            LOG_WARN("copy", "> Create this dir? [yes (default) / no]");

            std::string select;
            std::getline(std::cin, select);

            if (!select.empty() && select.substr(0, 1) != "y")
                return;

            fs::create_directory(binaryDir);
        }
    }
    catch (fs::filesystem_error const& error)
    {
        LOG_ERROR("copy", "> Error at check birary dir '{}': {}", binaryDir.generic_string(), error.what());
        return;
    }

    std::size_t filesCopied{ 0 };

    ProgressBar bar({}, _localeFileStorage.size());

    for (auto const& path : _localeFileStorage)
    {
        auto fileName = path.filename().generic_string();

        try
        {
            if (fs::copy_file(path, fs::path(binaryDir.generic_string() + fileName), fs::copy_options::update_existing))
                filesCopied++;

            bar.UpdatePostfixText(fileName);
            bar.Update();
        }
        catch (fs::filesystem_error const& error)
        {
            LOG_ERROR("copy", "> Error at copy file: {}", error.what());
        }
    }

    bar.Stop();

    if (!filesCopied)
        LOG_INFO("copy", "> All files is actual");

    LOG_INFO("copy", "--");
}

void CopyMgr::CopyScripts()
{
    LOG_INFO("copy", "> Prepare to copy dll scripts");

    auto const& sc = GetSC(_selectedSC);
    if (!sc)
    {
        LOG_ERROR("copy", "> Not found paths for SC '{}'", _selectedSC);
        return;
    }

    fs::path sourceDir{ sc->first };
    sourceDir += PATH_TO_BIN;
    sourceDir += "scripts/";

    fs::path binaryDir{ sc->second + "scripts/" };

    auto ClearBinaryDir = [this](fs::path const& path) -> std::optional<size_t>
    {
        ListExtensions extensions{ ".dll", ".pdb" };
        if (!FillFileList(path, &extensions))
            return {};

        std::size_t count{ 0 };

        for (auto const& path : _localeFileStorage)
            if (fs::remove(path))
                count++;

        return count;
    };

    try
    {
        if (!fs::exists(sourceDir))
        {
            LOG_WARN("copy", "> Dir '{}' don't exist. Skip this step", sourceDir.generic_string());
            return;
        }

        if (fs::is_empty(sourceDir))
        {
            LOG_WARN("copy", "> Dir '{}' is empty. Delete all scripts from binary path", sourceDir.generic_string());
            LOG_INFO("copy", "> Deleted {} files", ClearBinaryDir(binaryDir).value_or(0));
            return;
        }
    }
    catch (fs::filesystem_error const& error)
    {
        LOG_ERROR("copy", "> Error at check birary dir '{}': {}", binaryDir.generic_string(), error.what());
        return;
    }

    LOG_INFO("copy", "> Clear scripts before copy");
    LOG_INFO("copy", "> Deleted {} files", ClearBinaryDir(binaryDir).value_or(0));

    ListExtensions extensions{ ".dll", ".pdb" };
    if (!FillFileList(sourceDir, &extensions))
    {
        LOG_ERROR("copy", "> Skip this step");
        return;
    }

    if (_localeFileStorage.empty())
        return;

    LOG_DEBUG("copy", "> Start copy dll scripts");
    LOG_DEBUG("copy", "--");
    LOG_DEBUG("copy", "> Found {} scripts", _localeFileStorage.size() / 2);
    LOG_DEBUG("copy", "> From: {}. To: {}", sourceDir.generic_string(), binaryDir.generic_string());
    LOG_DEBUG("copy", "--");

    try
    {
        if (!fs::exists(binaryDir))
        {
            LOG_ERROR("copy", "> Dir '{}' don't exist", binaryDir.generic_string());
            LOG_WARN("copy", "> Create this dir? [yes (default) / no]");

            std::string select;
            std::getline(std::cin, select);

            if (!select.empty() && select.substr(0, 1) != "y")
                return;

            fs::create_directory(binaryDir);
        }
    }
    catch (fs::filesystem_error const& error)
    {
        LOG_ERROR("copy", "> Error at check birary dir '{}': {}", binaryDir.generic_string(), error.what());
        return;
    }

    std::size_t filesCopied{ 0 };

    ProgressBar bar({}, _localeFileStorage.size());

    for (auto const& path : _localeFileStorage)
    {
        auto fileName = path.filename().generic_string();

        try
        {
            if (fs::copy_file(path, fs::path(binaryDir.generic_string() + fileName), fs::copy_options::update_existing))
                filesCopied++;

            bar.UpdatePostfixText(fileName);
            bar.Update();
        }
        catch (fs::filesystem_error const& error)
        {
            LOG_ERROR("copy", "> Error at copy file: {}", error.what());
        }
    }

    bar.Stop();

    if (!filesCopied)
        LOG_INFO("copy", "> All files is actual");

    LOG_INFO("copy", "--");
}
