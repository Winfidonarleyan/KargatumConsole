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

#ifndef _DATABASE_MGR_H_
#define _DATABASE_MGR_H_

#include "Define.h"
#include "Duration.h"
#include <atomic>
#include <memory>
#include <vector>
#include <unordered_map>

namespace Warhead::Database
{
    class DatabaseWorkerPool;

    class WH_DATABASE_API DatabaseThread
    {
    public:
        DatabaseThread() = default;
        ~DatabaseThread() = default;

        static void StopNow() { _stopEvent = true; }
        static bool IsStopped() { return _stopEvent; }

        static void StartThread();
        static void StopThread();

    private:
        // Thread
        static std::atomic<bool> _stopEvent;

        DatabaseThread(DatabaseThread const&) = delete;
        DatabaseThread(DatabaseThread&&) = delete;
        DatabaseThread& operator=(DatabaseThread const&) = delete;
        DatabaseThread& operator=(DatabaseThread&&) = delete;
    };

    class WH_DATABASE_API DatabaseMgr
    {
    public:
        DatabaseMgr();
        ~DatabaseMgr();

        static DatabaseMgr* instance();

        void AddNewPool(uint32 index, std::string_view connectionInfo);
        bool OpenConnections();
        DatabaseWorkerPool* GetDBPool(uint32 index);

    private:
        std::unordered_map<uint32, std::unique_ptr<DatabaseWorkerPool>> _poolList;

        DatabaseMgr(DatabaseMgr const&) = delete;
        DatabaseMgr(DatabaseMgr&&) = delete;
        DatabaseMgr& operator=(DatabaseMgr const&) = delete;
        DatabaseMgr& operator=(DatabaseMgr&&) = delete;
    };
}

#define sDatabaseMgr Warhead::Database::DatabaseMgr::instance()
#define LoginDatabase Warhead::Database::DatabaseMgr::instance()->GetDBPool(1)

#endif
