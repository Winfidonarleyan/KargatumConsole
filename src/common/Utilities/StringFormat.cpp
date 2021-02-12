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
#include <Poco/Exception.h>
#include <Poco/RegularExpression.h>
#include <Poco/String.h>

std::string Warhead::String::Trim(std::string& str)
{
    return Poco::trim(str);
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
    return Poco::trimRightInPlace(str);
}

std::string Warhead::String::Replace(std::string& str, std::string const& from, std::string const& to)
{
    return Poco::replace(str, from, to);
}

std::string Warhead::String::ReplaceInPlace(std::string& str, std::string const& from, std::string const& to)
{
    return Poco::replaceInPlace(str, from, to);
}

uint32 Warhead::String::PatternReplace(std::string& subject, const std::string& replacementFrom, const std::string& replacementTo)
{
    Poco::RegularExpression re(replacementFrom, Poco::RegularExpression::RE_MULTILINE);
    return re.subst(subject, replacementTo, Poco::RegularExpression::RE_GLOBAL);
}
