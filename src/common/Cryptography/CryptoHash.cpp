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

#include "CryptoHash.h"
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <fstream>

std::string Warhead::Crypto::GetMD5HashFromFile(std::string const& filePath)
{
    using Poco::DigestOutputStream;
    using Poco::DigestEngine;
    using Poco::MD5Engine;

    std::ifstream in(filePath, std::ios_base::binary);

    MD5Engine md5;
    DigestOutputStream ostr(md5);
    ostr << in.rdbuf();
    ostr.flush(); // Ensure everything gets passed to the digest engine

    const DigestEngine::Digest& digest = md5.digest(); // obtain result

    return DigestEngine::digestToHex(digest);
}
