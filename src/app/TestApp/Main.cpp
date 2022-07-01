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

#include "Common.h"
#include "Log.h"
#include "Duration.h"
#include "Config.h"
#include <chrono>
#include <format>
#include <filesystem>

int main()
{
    auto configFile = std::filesystem::path("TestApp.conf");

    // Add file and args in config
    sConfigMgr->Configure(configFile.generic_string());

    if (!sConfigMgr->LoadAppConfigs())
        return 1;

    // Init logging
    //sLog->Initialize();
    sLog->UsingDefaultLogs();

    LOG_INFO("server", "");
    LOG_INFO("server", " ██╗     ██╗  █████╗  ██████╗  ██╗  ██╗ ███████╗  █████╗  ██████╗");
    LOG_INFO("server", " ██║     ██║ ██╔══██╗ ██╔══██╗ ██║  ██║ ██╔════╝ ██╔══██╗ ██╔═══██╗");
    LOG_INFO("server", " ██║     ██║ ███████║ ██████╔╝ ███████║ █████╗   ███████║ ██║   ██║");
    LOG_INFO("server", " ██║ ██╗ ██║ ██╔══██║ ██╔══██╗ ██╔══██║ ██╔══╝   ██╔══██║ ██║   ██║");
    LOG_INFO("server", " ╚═██╔═██╔═╝ ██║  ██║ ██║  ██║ ██║  ██║ ███████╗ ██║  ██║ ██████╔═╝");
    LOG_INFO("server", "   ╚═╝ ╚═╝   ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚══════╝ ╚═╝  ╚═╝ ╚═════╝");
    LOG_INFO("server", "                               ██████╗   ██████╗  ██████╗  ███████╗");
    LOG_INFO("server", "                               ██╔═══╝  ██╔═══██╗ ██╔══██╗ ██╔════╝");
    LOG_INFO("server", "                               ██║      ██║   ██║ ██████╔╝ █████╗");
    LOG_INFO("server", "                               ██║      ██║   ██║ ██╔══██╗ ██╔══╝");
    LOG_INFO("server", "                               ╚██████╗ ╚██████╔╝ ██║  ██║ ███████╗");
    LOG_INFO("server", "                                ╚═════╝  ╚═════╝  ╚═╝  ╚═╝ ╚══════╝");
    LOG_INFO("server", "");

    LOG_GM(1, "Test gm");

    //std::chrono::zoned_time cur_time{ std::chrono::current_zone(), std::chrono::system_clock::now() };

    //LOG_INFO("server", "Seconds - {}", std::format("{}", std::chrono::duration_cast<Seconds>(cur_time.get_local_time().time_since_epoch())));

    return 0;
}
