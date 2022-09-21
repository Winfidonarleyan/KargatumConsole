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

#include "Field.h"
#include "Errors.h"
#include "Log.h"
#include "StringConvert.h"
#include "Types.h"

namespace
{
    template<typename T>
    constexpr T GetDefaultValue()
    {
        if constexpr (std::is_same_v<T, bool>)
            return false;
        else if constexpr (std::is_integral_v<T>)
            return 0;
        else if constexpr (std::is_floating_point_v<T>)
            return 1.0f;
        else if constexpr (std::is_same_v<T, std::vector<uint8>> || std::is_same_v<std::string_view, T>)
            return {};
        else
            return "";
    }

    template<typename T>
    inline bool IsCorrectFieldType(Warhead::Database::DatabaseFieldTypes type)
    {
        // Int8
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int8> || std::is_same_v<T, uint8>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Int8)
                return true;
        }

        // In16
        if constexpr (std::is_same_v<T, uint16> || std::is_same_v<T, int16>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Int16)
                return true;
        }

        // Int32
        if constexpr (std::is_same_v<T, uint32> || std::is_same_v<T, int32>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Int32)
                return true;
        }

        // Int64
        if constexpr (std::is_same_v<T, uint64> || std::is_same_v<T, int64>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Int64)
                return true;
        }

        // float
        if constexpr (std::is_same_v<T, float>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Float)
                return true;
        }

        // double
        if constexpr (std::is_same_v<T, double>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Double || type == Warhead::Database::DatabaseFieldTypes::Decimal)
                return true;
        }

        // Binary
        if constexpr (std::is_same_v<T, Warhead::Database::Binary>)
        {
            if (type == Warhead::Database::DatabaseFieldTypes::Binary)
                return true;
        }

        return false;
    }

    inline std::optional<std::string_view> GetCleanAliasName(std::string_view alias)
    {
        if (alias.empty())
            return {};

        auto pos = alias.find_first_of('(');
        if (pos == std::string_view::npos)
            return {};

        alias.remove_suffix(alias.length() - pos);

        return { alias };
    }

    template<typename T>
    inline bool IsCorrectAlias(Warhead::Database::DatabaseFieldTypes type, std::string_view alias)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            if ((StringEqualI(alias, "sum") || StringEqualI(alias, "avg")) && type == Warhead::Database::DatabaseFieldTypes::Decimal)
                return true;

            return false;
        }

        if constexpr (std::is_same_v<T, uint64>)
        {
            if (StringEqualI(alias, "count") && type == Warhead::Database::DatabaseFieldTypes::Int64)
                return true;

            return false;
        }

        if ((StringEqualI(alias, "min") || StringEqualI(alias, "max")) && IsCorrectFieldType<T>(type))
        {
            return true;
        }

        return false;
    }
}

Warhead::Database::Field::Field()
{
    data.value = nullptr;
    data.length = 0;
    data.raw = false;
    meta = nullptr;
}

void Warhead::Database::Field::GetBinarySizeChecked(uint8* buf, size_t length) const
{
    ASSERT(data.value && (data.length == length), "Expected {}-byte binary blob, got {}data ({} bytes) instead", length, data.value ? "" : "no ", data.length);
    memcpy(buf, data.value, length);
}

void Warhead::Database::Field::SetByteValue(char const* newValue, uint32 length)
{
    // This value stores raw bytes that have to be explicitly cast later
    data.value = newValue;
    data.length = length;
    data.raw = true;
}

void Warhead::Database::Field::SetStructuredValue(char const* newValue, uint32 length)
{
    // This value stores somewhat structured data that needs function style casting
    data.value = newValue;
    data.length = length;
    data.raw = false;
}

bool Warhead::Database::Field::IsType(DatabaseFieldTypes type) const
{
    return meta->Type == type;
}

bool Warhead::Database::Field::IsNumeric() const
{
    return (meta->Type == DatabaseFieldTypes::Int8 ||
        meta->Type == DatabaseFieldTypes::Int16 ||
        meta->Type == DatabaseFieldTypes::Int32 ||
        meta->Type == DatabaseFieldTypes::Int64 ||
        meta->Type == DatabaseFieldTypes::Float ||
        meta->Type == DatabaseFieldTypes::Double);
}

void Warhead::Database::Field::LogWrongType(std::string_view getter, std::string_view typeName) const
{
    LOG_WARN("sql.sql", "Warning: {}<{}> on {} field {}.{} ({}.{}) at index {}.",
        getter, typeName, meta->TypeName, meta->TableAlias, meta->Alias, meta->TableName, meta->Name, meta->Index);
}

void Warhead::Database::Field::SetMetadata(QueryResultFieldMetadata const* fieldMeta)
{
    meta = fieldMeta;
}

template<typename T>
T Warhead::Database::Field::GetData() const
{
    static_assert(std::is_arithmetic_v<T>, "Unsurropt type for Field::GetData()");

    if (!data.value)
        return GetDefaultValue<T>();

#ifdef WARHEAD_STRICT_DATABASE_TYPE_CHECKS
    if (!IsCorrectFieldType<T>(meta->Type))
    {
        LogWrongType(__FUNCTION__, typeid(T).name());
        //return GetDefaultValue<T>();
    }
#endif

    std::optional<T> result = {};

    if (data.raw)
        result = *reinterpret_cast<T const*>(data.value);
    else
        result = Warhead::StringTo<T>(data.value);

    // Correct double fields... this undefined behavior :/
    if constexpr (std::is_same_v<T, double>)
    {
        if (data.raw && !IsType(DatabaseFieldTypes::Decimal))
            result = *reinterpret_cast<double const*>(data.value);
        else
            result = Warhead::StringTo<float>(data.value);
    }

    // Check -1 for *_dbc db tables
    if constexpr (std::is_same_v<T, uint32>)
    {
        std::string_view tableName{ meta->TableName };

        if (!tableName.empty() && tableName.size() > 4)
        {
            auto signedResult = Warhead::StringTo<int32>(data.value);

            if (signedResult && !result && tableName.substr(tableName.length() - 4) == "_dbc")
            {
                LOG_DEBUG("sql.sql", "> Found incorrect value '{}' for type '{}' in _dbc table.", data.value, typeid(T).name());
                LOG_DEBUG("sql.sql", "> Table name '{}'. Field name '{}'. Try return int32 value", meta->TableName, meta->Name);
                return GetData<int32>();
            }
        }
    }

    if (auto alias = GetCleanAliasName(meta->Alias))
    {
        if ((StringEqualI(*alias, "min") || StringEqualI(*alias, "max")) && !IsCorrectAlias<T>(meta->Type, *alias))
        {
            LogWrongType(__FUNCTION__, typeid(T).name());
            //ABORT();
        }

        if ((StringEqualI(*alias, "sum") || StringEqualI(*alias, "avg")) && !IsCorrectAlias<T>(meta->Type, *alias))
        {
            LogWrongType(__FUNCTION__, typeid(T).name());
            LOG_WARN("sql.sql", "> Please use GetData<double>()");
            return GetData<double>();
            //ABORT();
        }

        if (StringEqualI(*alias, "count") && !IsCorrectAlias<T>(meta->Type, *alias))
        {
            LogWrongType(__FUNCTION__, typeid(T).name());
            LOG_WARN("sql.sql", "> Please use GetData<uint64>()");
            return GetData<uint64>();
            //ABORT();
        }
    }

    if (!result)
    {
        LOG_FATAL("sql.sql", "> Incorrect value '{}' for type '{}'. Value is raw ? '{}'", data.value, typeid(T).name(), data.raw);
        LOG_FATAL("sql.sql", "> Table name '{}'. Field name '{}'", meta->TableName, meta->Name);
        //ABORT();
        return GetDefaultValue<T>();
    }

    return *result;
}

template WH_DATABASE_API bool Warhead::Database::Field::GetData() const;
template WH_DATABASE_API uint8 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API uint16 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API uint32 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API uint64 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API int8 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API int16 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API int32 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API int64 Warhead::Database::Field::GetData() const;
template WH_DATABASE_API float Warhead::Database::Field::GetData() const;
template WH_DATABASE_API double Warhead::Database::Field::GetData() const;

std::string Warhead::Database::Field::GetDataString() const
{
    if (!data.value)
        return "";

#ifdef WARHEAD_STRICT_DATABASE_TYPE_CHECKS
    if (IsNumeric() && data.raw)
    {
        LogWrongType(__FUNCTION__, "std::string");
        return "";
    }
#endif

    return { data.value, data.length };
}

std::string_view Warhead::Database::Field::GetDataStringView() const
{
    if (!data.value)
        return {};

#ifdef WARHEAD_STRICT_DATABASE_TYPE_CHECKS
    if (IsNumeric() && data.raw)
    {
        LogWrongType(__FUNCTION__, "std::string_view");
        return {};
    }
#endif

    return { data.value, data.length };
}

Warhead::Database::Binary Warhead::Database::Field::GetDataBinary() const
{
    Binary result = {};
    if (!data.value || !data.length)
        return result;

#ifdef WARHEAD_STRICT_DATABASE_TYPE_CHECKS
    if (!IsCorrectFieldType<Binary>(meta->Type))
    {
        LogWrongType(__FUNCTION__, "Binary");
        return {};
    }
#endif

    result.resize(data.length);
    memcpy(result.data(), data.value, data.length);
    return result;
}
