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

#ifndef DatabaseEnvFwd_h__
#define DatabaseEnvFwd_h__

#include "Define.h"
#include <future>
#include <memory>
#include <vector>

namespace Warhead::Database
{
    enum class ConnectionFlags : uint8
    {
        Async   = 0x1,
        Sync    = 0x2,
        Both    = Async | Sync
    };

    class Field;
    class MySQLConnection;
    class MySQLPreparedStatement;
    class PreparedStatement;

    struct MySQLConnectionInfo;
    struct QueryResultFieldMetadata;

    class ResultSet;
    using QueryResult = std::shared_ptr<ResultSet>;
    using QueryResultFuture = std::future<QueryResult>;
    using QueryResultPromise = std::promise<QueryResult>;

    class PreparedResultSet;
    using PreparedQueryResult = std::shared_ptr<PreparedResultSet>;
    using PreparedQueryResultFuture = std::future<PreparedQueryResult>;
    using PreparedQueryResultPromise = std::promise<PreparedQueryResult>;
}

// mysql
struct MySQLHandle;
struct MySQLResult;
struct MySQLField;
struct MySQLBind;
struct MySQLStmt;

#endif // DatabaseEnvFwd_h__
