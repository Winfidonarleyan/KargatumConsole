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

#include "StringFormat.h"
#include "Log.h"
#include <Poco/String.h>
#include <Warhead/RegularExpression.h>
#include <Warhead/RegularExpressionException.h>
#include <locale>

std::string Warhead::String::Trim(std::string& str)
{
    return Poco::trim(str);
}

template<class Str>
WH_COMMON_API Str Warhead::String::Trim(const Str& s, const std::locale& loc /*= std::locale()*/)
{
    typename Str::const_iterator first = s.begin();
    typename Str::const_iterator end = s.end();

    while (first != end && std::isspace(*first, loc))
        ++first;

    if (first == end)
        return Str();

    typename Str::const_iterator last = end;

    do
        --last;
    while (std::isspace(*last, loc));

    if (first != s.begin() || last + 1 != end)
        return Str(first, last + 1);

    return s;
}

std::string Warhead::String::TrimLeft(std::string& str)
{
    return Poco::trimLeft(str);
}

std::string Warhead::String::TrimLeftInPlace(std::string& str)
{
    return Poco::trimLeftInPlace(str);
}

std::string Warhead::String::TrimRight(std::string& str)
{
    return Poco::trimRight(str);
}

std::string Warhead::String::TrimRightInPlace(std::string& str)
{
    //return Poco::trimRightInPlace(str);

    int pos = int(str.size()) - 1;

    while (pos >= 0 && std::isspace(str[pos]))
    {
        --pos;
    }

    str.resize(pos + 1);

    return str;
}

std::string Warhead::String::Replace(std::string& str, std::string const& from, std::string const& to)
{
    return Poco::replace(str, from, to);
}

std::string Warhead::String::ReplaceInPlace(std::string& str, std::string const& from, std::string const& to)
{
    return Poco::replaceInPlace(str, from, to);
}

uint32 Warhead::String::PatternReplace(std::string& subject, std::string_view pattern, std::string_view replacement)
{
    try
    {
        Warhead::RegularExpression re(pattern, Warhead::RegularExpression::RE_MULTILINE);
        return re.subst(subject, replacement, Warhead::RegularExpression::RE_GLOBAL);
    }
    catch (const Warhead::RegularExpressionException& e)
    {
        LOG_FATAL("util", "> Warhead::String::PatternReplace: {}", e.GetErrorMessage());
    }

    return 0;
}

// Template Trim
template WH_COMMON_API std::string Warhead::String::Trim<std::string>(const std::string& s, const std::locale& loc /*= std::locale()*/);

//template<typename... Args>
//void Warhead::Impl::Print::For<Args...>::PrintFormat(std::string_view fmt, Args&& ...args)
//{
//    fmt::print(fmt, std::forward<Args>(args)...);
//}
