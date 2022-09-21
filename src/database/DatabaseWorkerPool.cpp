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

#include "DatabaseWorkerPool.h"
#include "AsyncCallbackMgr.h"
#include "Errors.h"
#include "Log.h"
#include "MySQLWorkaround.h"
#include "QueryResult.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "PreparedStatement.h"
#include <limits>

constexpr auto MAX_SYNC_CONNECTIONS = 32;

#ifdef WARHEAD_DEBUG
#include <boost/stacktrace.hpp>
#include <sstream>
#endif

#if MARIADB_VERSION_ID >= 100600
#define MIN_MYSQL_SERVER_VERSION 100300u
#define MIN_MYSQL_CLIENT_VERSION 30203u
#else
#define MIN_MYSQL_SERVER_VERSION 50700u
#define MIN_MYSQL_CLIENT_VERSION 50700u
#endif

Warhead::Database::DatabaseWorkerPool::DatabaseWorkerPool()
{
    WPFatal(mysql_thread_safe(), "Used MySQL library isn't thread-safe.");

#if !defined(MARIADB_VERSION_ID) || MARIADB_VERSION_ID < 100600
    bool isSupportClientDB = mysql_get_client_version() >= MIN_MYSQL_CLIENT_VERSION;
    bool isSameClientDB = mysql_get_client_version() == MYSQL_VERSION_ID;
#else // MariaDB 10.6+
    bool isSupportClientDB = mysql_get_client_version() >= MIN_MYSQL_CLIENT_VERSION;
    bool isSameClientDB    = true; // Client version 3.2.3?
#endif

    WPFatal(isSupportClientDB, "WarheadCore does not support MySQL versions below 5.7 and MariaDB 10.3\nSearch the wiki for ACE00043 in Common Errors (https://www.azerothcore.org/wiki/common-errors).");
    WPFatal(isSameClientDB, "Used MySQL library version ({} id {}) does not match the version id used to compile WarheadCore (id {}).\nSearch the wiki for ACE00046 in Common Errors (https://www.azerothcore.org/wiki/common-errors).",
        mysql_get_client_info(), mysql_get_client_version(), MYSQL_VERSION_ID);
}

void Warhead::Database::DatabaseWorkerPool::SetConnectionInfo(std::string_view infoString)
{
    _connectionInfo = std::make_unique<MySQLConnectionInfo>(infoString);
}

uint32 Warhead::Database::DatabaseWorkerPool::Open()
{
    ASSERT(_connectionInfo.get(), "Connection info was not set!");

    LOG_INFO("sql.driver", "Opening DatabasePool '{}'", GetDatabaseName());

    // Async connection
    {
        auto [error, connection] = OpenConnection(IDX_ASYNC);
        if (error)
            return error;
    }

    // Sync connection
    auto [error, connection] = OpenConnection(IDX_SYNCH);
    if (!error)
    {
        LOG_INFO("sql.driver", "DatabasePool '{}' opened successfully", GetDatabaseName());
    }

    LOG_INFO("sql.sql", "MySQL client library: {}", connection->GetClientInfo());
    LOG_INFO("sql.sql", "MySQL server ver: {} ", connection->GetServerInfo());
    LOG_INFO("sql.driver", "");
    return error;
}

void Warhead::Database::DatabaseWorkerPool::Close()
{
    LOG_INFO("sql.driver", "Closing down DatabasePool '{}'.", GetDatabaseName());

    //! Closes the actually MySQL connection.
    _asyncConnection.reset();

    LOG_INFO("sql.driver", "Asynchronous connections on DatabasePool '{}' terminated. Proceeding with synchronous connections.",
        GetDatabaseName());

    //! Shut down the synchronous connections
    //! There's no need for locking the connection, because DatabaseWorkerPool<>::Close
    //! should only be called after any other thread tasks in the core have exited,
    //! meaning there can be no concurrent access at this point.
    _connections.clear();

    LOG_INFO("sql.driver", "All connections on DatabasePool '{}' closed.", GetDatabaseName());
}

Warhead::Database::QueryResult Warhead::Database::DatabaseWorkerPool::Query(std::string_view sql)
{
    auto connection = GetFreeConnection();
    if (!connection)
        return { nullptr };

    auto result = connection->Query(sql);
    connection->Unlock();

    if (!result || !result->GetRowCount() || !result->NextRow())
    {
        delete result;
        return { nullptr };
    }

    return QueryResult(result);
}

std::pair<uint32, Warhead::Database::MySQLConnection*> Warhead::Database::DatabaseWorkerPool::OpenConnection(InternalIndex type, bool isDynamic /*= false*/)
{
    if (type == IDX_ASYNC)
    {
        _asyncConnection = std::make_unique<MySQLConnection>(*_connectionInfo, true);
        if (uint32 error = _asyncConnection->Open())
        {
            // Failed to open a connection or invalid version
            return { error, nullptr };
        }

        return { 0, _asyncConnection.get() };
    }
    else if (type == IDX_SYNCH)
    {
        auto connection = std::make_unique<MySQLConnection>(*_connectionInfo);
        if (uint32 error = connection->Open())
        {
            // Failed to open a connection or invalid version
            return { error, nullptr };
        }

        if (isDynamic)
            connection->SetDynamic();

        auto& itrConnection = _connections.emplace_back(std::move(connection));

        // Everything is fine
        return { 0, itrConnection.get() };
    }
    else
        ABORT("> Unknown connection type: {}", type);

    return { 0, nullptr };
}

Warhead::Database::MySQLConnection* Warhead::Database::DatabaseWorkerPool::GetFreeConnection()
{
#ifdef WARHEAD_DEBUG
    if (_warnSyncQueries)
    {
        std::ostringstream ss;
        ss << boost::stacktrace::stacktrace();
        LOG_WARN("sql.performances", "Sync query at:\n{}", ss.str());
    }
#endif

    std::lock_guard<std::mutex> guard(_cleanupMutex);

    // Check default connections
    for (auto& connection : _connections)
        if (connection->LockIfReady())
            return connection.get();

    // Try to make new connect if connections count < MAX_SYNC_CONNECTIONS
    {
        std::lock_guard<std::mutex> guard(_openConnectMutex);

        LOG_WARN("sql", "> Not found free sync connection. Size: {}", _connections.size());

        if (_connections.size() < MAX_SYNC_CONNECTIONS)
        {
            auto [error, connection] = OpenConnection(IDX_SYNCH, true);
            if (!error)
            {
                InitPrepareStatement(connection);

                if (connection->PrepareStatements() && connection->LockIfReady());
                    return connection;
            }
        }
    }

    MySQLConnection* freeConnection{ nullptr };

    uint8 i{};
    auto const num_cons{ _connections.size()};

    //! Block forever until a connection is free
    for (;;)
    {
        auto index = ++i % num_cons;
        freeConnection = _connections[index].get();

        //! Must be matched with t->Unlock() or you will get deadlocks
        if (freeConnection->LockIfReady())
            break;
    }

    return freeConnection;
}

std::string_view Warhead::Database::DatabaseWorkerPool::GetDatabaseName() const
{
    return std::string_view{ _connectionInfo->Database };
}

void Warhead::Database::DatabaseWorkerPool::Execute(std::string_view sql)
{
    if (sql.empty())
        return;

    sAsyncCallbackMgr->AddAsyncCallback([this, sql = std::string(sql)]()
    {
        _asyncConnection->Execute(sql);
    });
}

void Warhead::Database::DatabaseWorkerPool::CleanupConnections()
{
    std::lock_guard<std::mutex> guard(_cleanupMutex);

    _connections.erase(std::remove_if(_connections.begin(), _connections.end(), [](std::unique_ptr<MySQLConnection>& connection)
    {
        return connection->IsDynamic();
    }), _connections.end());
}

bool Warhead::Database::DatabaseWorkerPool::PrepareStatements()
{
    for (auto const& [index, stmt] : _stringPreparedStatement)
    {
        for (auto& connection : _connections)
            connection->PrepareStatement(index, stmt.Query, stmt.ConnectionType);

        _asyncConnection->PrepareStatement(index, stmt.Query, stmt.ConnectionType);
    }

    auto IsValidPrepareStatements = [this](MySQLConnection* connection)
    {
        if (!connection->LockIfReady())
            return false;

        if (!connection->PrepareStatements())
        {
            connection->Unlock();
            Close();
            return false;
        }
        else
            connection->Unlock();

        auto list = connection->GetPreparedStatementList();
        auto const preparedSize = list->size();

        if (_preparedStatementSize.size() < preparedSize)
            _preparedStatementSize.resize(preparedSize);

        for (uint32 i = 0; i < preparedSize; ++i)
        {
            // already set by another connection
            // (each connection only has prepared statements of its own type sync/async)
            if (_preparedStatementSize[i] > 0)
                continue;

            auto const& itr = list->find(i);
            if (itr != list->end())
            {
                auto stmt = itr->second.get();
                if (stmt)
                {
                    uint32 const paramCount = stmt->GetParameterCount();

                    // WH only supports uint8 indices.
                    ASSERT(paramCount < std::numeric_limits<uint8>::max());

                    _preparedStatementSize[i] = static_cast<uint8>(paramCount);
                }
            }
        }

        return true;
    };

    for (auto const& syncConnection : _connections)
        if (!IsValidPrepareStatements(syncConnection.get()))
            return false;

    if (!IsValidPrepareStatements(_asyncConnection.get()))
        return false;

    return true;
}

void Warhead::Database::DatabaseWorkerPool::Execute(PreparedStatement* stmt)
{
    if (!stmt)
        return;

    sAsyncCallbackMgr->AddAsyncCallback([this, stmt]()
    {
        _asyncConnection->Execute(stmt);
        delete stmt;
    });
}

Warhead::Database::PreparedStatement* Warhead::Database::DatabaseWorkerPool::GetPreparedStatement(uint32 index)
{
    return new PreparedStatement(index, _preparedStatementSize[index]);
}

void Warhead::Database::DatabaseWorkerPool::PrepareStatement(uint32 index, std::string_view sql, Warhead::Database::ConnectionFlags flags)
{
    auto const& itr = _stringPreparedStatement.find(index);
    if (itr != _stringPreparedStatement.end())
    {
        LOG_ERROR("sql", "> Prepared statement with index () is exist! Skip", index);
        return;
    }

    _stringPreparedStatement.emplace(index, StringPreparedStatement{ index, sql, flags });
}

Warhead::Database::PreparedQueryResult Warhead::Database::DatabaseWorkerPool::Query(PreparedStatement* stmt)
{
    auto connection = GetFreeConnection();
    if (!connection)
        return { nullptr };

    auto result = connection->Query(stmt);
    connection->Unlock();

    //! Delete proxy-class. Not needed anymore
    delete stmt;

    if (!result || !result->GetRowCount())
    {
        delete result;
        return { nullptr };
    }

    return PreparedQueryResult(result);
}

void Warhead::Database::DatabaseWorkerPool::InitPrepareStatement(MySQLConnection* connection)
{
    for (auto const& [index, stmt] : _stringPreparedStatement)
        connection->PrepareStatement(index, stmt.Query, stmt.ConnectionType);
}

