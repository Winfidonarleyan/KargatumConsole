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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "AsyncCallbackMgr.h"
#include "DatabaseMgr.h"
#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"
#include "Timer.h"
#include <mysql.h>

std::atomic<bool> Warhead::Database::DatabaseThread::_stopEvent = false;

void Warhead::Database::DatabaseThread::StartThread()
{
    auto realCurrTime = 0ms;
    auto realPrevTime = GetTimeMS();

    // While we have not ServerMgr::_stopEvent, update the server
    while (!DatabaseThread::IsStopped())
    {
        realCurrTime = GetTimeMS();

        auto diff = GetMSTimeDiff(realPrevTime, realCurrTime);
        if (diff == 0ms)
        {
            // sleep until enough time passes that we can update all timers
            std::this_thread::sleep_for(1ms);
            continue;
        }

        sAsyncCallbackMgr->ProcessReadyCallbacks();
        realPrevTime = realCurrTime;
    }
}

void Warhead::Database::DatabaseThread::StopThread()
{
    DatabaseThread::StopNow();
}

Warhead::Database::DatabaseMgr::DatabaseMgr()
{
    mysql_library_init(0, nullptr, nullptr);
}

Warhead::Database::DatabaseMgr::~DatabaseMgr()
{
    mysql_library_end();
}

/*static*/ Warhead::Database::DatabaseMgr* Warhead::Database::DatabaseMgr::instance()
{
    static DatabaseMgr instance;
    return &instance;
}

void Warhead::Database::DatabaseMgr::AddNewPool(uint32 index, std::string_view connectionInfo)
{
    auto dbPool = std::make_unique<DatabaseWorkerPool>();
    dbPool->SetConnectionInfo(connectionInfo);
    _poolList.emplace(index, std::move(dbPool));
}

Warhead::Database::DatabaseWorkerPool* Warhead::Database::DatabaseMgr::GetDBPool(uint32 index)
{
    auto const& itr = _poolList.find(index);
    if (itr == _poolList.end())
        return nullptr;

    return itr->second.get();
}

bool Warhead::Database::DatabaseMgr::OpenConnections()
{
    for (auto& [index, dbPool] : _poolList)
        if (dbPool->Open())
            return false;

    return true;
}

