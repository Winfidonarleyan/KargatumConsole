/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
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
#include <Poco/String.h>

void Warhead::String::Trim(std::string& str)
{
    Poco::trim(str);
}

void Warhead::String::TrimLeft(std::string& str)
{
    Poco::trimLeft(str);
}

void Warhead::String::TrimLeftInPlace(std::string& str)
{
    Poco::trimLeftInPlace(str);
}

void Warhead::String::TrimRight(std::string& str)
{
    Poco::trimRight(str);
}

void Warhead::String::TrimRightInPlace(std::string& str)
{
    Poco::trimRightInPlace(str);
}
