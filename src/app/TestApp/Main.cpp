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
#include "Log.h"
#include "Duration.h"
#include "Config.h"
#include <chrono>
#include <filesystem>
#include "LaunchProcess.h"

int main()
{
    // Init logging
    sLog->UsingDefaultLogs();

    LOG_INFO("test", "Is? {}", Warhead::Process::Git::IsRepository("."));
    LOG_INFO("test", "Is? {}", Warhead::Process::Git::IsRepository("/home/winfidonarleyan"));

    return 0;
}
