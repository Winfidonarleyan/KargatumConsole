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

#ifndef _DATABASEWORKERPOOL_H
#define _DATABASEWORKERPOOL_H

#include "DatabaseEnvFwd.h"
#include "StringFormat.h"
#include "Duration.h"
#include <array>
#include <vector>
#include <mutex>
#include <unordered_map>

namespace Warhead::Database
{
    struct StringPreparedStatement
    {
        StringPreparedStatement(uint32 index, std::string_view sql, ConnectionFlags flags) :
            Index(index), Query(sql), ConnectionType(flags) { }

        uint32 Index{};
        std::string Query;
        ConnectionFlags ConnectionType{ ConnectionFlags::Sync };
    };

    class WH_DATABASE_API DatabaseWorkerPool
    {
    private:
        enum InternalIndex
        {
            IDX_ASYNC,
            IDX_SYNCH,
            IDX_SIZE
        };

    public:
        /* Activity state */
        DatabaseWorkerPool();
        ~DatabaseWorkerPool() = default;

        void SetConnectionInfo(std::string_view infoString);

        uint32 Open();
        void Close();

        //! Prepares all prepared statements
        bool PrepareStatements();

        [[nodiscard]] inline MySQLConnectionInfo const* GetConnectionInfo() const
        {
            return _connectionInfo.get();
        }

        /**
            Delayed one-way statement methods.
        */

        //! Enqueues a one-way SQL operation in string format that will be executed asynchronously.
        //! This method should only be used for queries that are only executed once, e.g during startup.
        void Execute(std::string_view sql);

        //! Enqueues a one-way SQL operation in string format -with variable args- that will be executed asynchronously.
        //! This method should only be used for queries that are only executed once, e.g during startup.
        template<typename... Args>
        void Execute(std::string_view sql, Args&&... args)
        {
            if (sql.empty())
                return;

            Execute(Warhead::StringFormat(sql, std::forward<Args>(args)...));
        }

        void Execute(PreparedStatement* stmt);

        //! Directly executes an SQL query in string format that will block the calling thread until finished.
        //! Returns reference counted auto pointer, no need for manual memory management in upper level code.
        QueryResult Query(std::string_view sql);

        //! Directly executes an SQL query in string format -with variable args- that will block the calling thread until finished.
        //! Returns reference counted auto pointer, no need for manual memory management in upper level code.
        template<typename... Args>
        QueryResult Query(std::string_view sql, Args&&... args)
        {
            if (sql.empty())
                return { nullptr };

            return Query(Warhead::StringFormat(sql, std::forward<Args>(args)...));
        }

        PreparedQueryResult Query(PreparedStatement* stmt);

        //! Automanaged (internally) pointer to a prepared statement object for usage in upper level code.
        //! Pointer is deleted in this->DirectExecute(PreparedStatement*), this->Query(PreparedStatement*) or PreparedStatementTask::~PreparedStatementTask.
        //! This object is not tied to the prepared statement on the MySQL context yet until execution.
        PreparedStatement* GetPreparedStatement(uint32 index);

        void PrepareStatement(uint32 index, std::string_view sql, ConnectionFlags flags);

        void CleanupConnections();

    private:
        std::pair<uint32, MySQLConnection*> OpenConnection(InternalIndex type, bool isDynamic = false);

        void InitPrepareStatement(MySQLConnection* connection);

        //! Gets a free connection in the synchronous connection pool.
        //! Caller MUST call t->Unlock() after touching the MySQL context to prevent deadlocks.
        MySQLConnection* GetFreeConnection();

        [[nodiscard]] std::string_view GetDatabaseName() const;

        std::unordered_map<uint32, StringPreparedStatement> _stringPreparedStatement;
        std::vector<std::unique_ptr<MySQLConnection>> _connections;
        std::unique_ptr<MySQLConnection> _asyncConnection;
        std::unique_ptr<MySQLConnectionInfo> _connectionInfo;
        std::vector<uint8> _preparedStatementSize;
        std::mutex _openConnectMutex;
        std::mutex _cleanupMutex;

#ifdef WARHEAD_DEBUG
        static inline thread_local bool _warnSyncQueries = false;
#endif
    };
}

#endif
