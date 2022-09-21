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

#include "Log.h"
#include "MySQLConnection.h"
#include "QueryResult.h"
#include "DatabaseWorkerPool.h"
#include "DatabaseMgr.h"
#include "StopWatch.h"
#include "PreparedStatement.h"
#include <thread>
#include <memory>

void TestQuery()
{
//    LoginDatabase->Execute("UPDATE wh_auth.realmlist t SET t.name = 'WarheadCore1' WHERE t.id = 1");

    StopWatch sw;

    auto result = LoginDatabase->Query("SELECT path, state FROM updates_include");
    if (!result)
    {
        LOG_ERROR("test", "!res");
        return;
    }

    LOG_INFO("test", "> Elapsed: {}. Row count: {}", sw, result->GetRowCount());
};

void TestStmt()
{
    StopWatch sw;

    auto result = LoginDatabase->Query(LoginDatabase->GetPreparedStatement(0));
    if (!result)
    {
        LOG_ERROR("test", "!res");
        return;
    }

    LOG_INFO("test", "> Elapsed: {}. Row count: {}", sw, result->GetRowCount());
}

void MakeThreads(std::size_t count)
{
    std::vector<std::thread> pool;

    for (std::size_t i{}; i < count; i++)
        pool.emplace_back(&TestStmt);

    for (auto& itr : pool)
        itr.join();
}

int main()
{
    // Init logging
    sLog->UsingDefaultLogs();

    using namespace Warhead::Database;

    // Add pool
    sDatabaseMgr->AddNewPool(1, "127.0.0.1;3306;warhead;warhead;wh_auth");

    // Add PS
    LoginDatabase->PrepareStatement(0, "SELECT id, username FROM account", ConnectionFlags::Sync);
    LoginDatabase->PrepareStatement(1, "SELECT id, username FROM account WHERE username = ?", ConnectionFlags::Sync);
    LoginDatabase->PrepareStatement(2, "UPDATE wh_auth.realmlist t SET t.name = ? WHERE t.id = ?", ConnectionFlags::Async);

    // Open connect
    sDatabaseMgr->OpenConnections();

    // Check PS
    if (!LoginDatabase->PrepareStatements())
        return 0;

    MakeThreads(1);

    LoginDatabase->CleanupConnections();

    MakeThreads(20);
    MakeThreads(5);

    LoginDatabase->CleanupConnections();

    MakeThreads(5);

//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();
//    TestQuery();

//    Warhead::Database::DatabaseThread::StartThread();

    LOG_INFO("test", ">> Wait");

    std::this_thread::sleep_for(3s);
    return 0;
}
