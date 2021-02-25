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

#include "Util.h"
#include "Common.h"
#include "Log.h"
#include <Poco/Path.h>
#include <Poco/File.h>
#include <filesystem>
#include <fstream>

std::vector<std::string_view> Warhead::Tokenize(std::string_view str, char sep, bool keepEmpty)
{
    std::vector<std::string_view> tokens;

    size_t start = 0;
    for (size_t end = str.find(sep); end != std::string_view::npos; end = str.find(sep, start))
    {
        if (keepEmpty || (start < end))
            tokens.push_back(str.substr(start, end - start));

        start = end + 1;
    }

    if (keepEmpty || (start < str.length()))
        tokens.push_back(str.substr(start));

    return tokens;
}

bool StringEqualI(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

std::string Warhead::File::GetFileText(std::string const& path, bool openBinary /*= false*/)
{
    std::filesystem::path _path(path);

    int openMode = std::ios_base::in;

    if (openBinary)
        openMode |= std::ios_base::binary;

    std::ifstream in(_path, openMode);

    if (!in.is_open())
    {
        LOG_FATAL("> Failed to open file (%s)", _path.generic_string().c_str());
        return "";
    }

    auto text = [&in]
    {
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }();

    in.close();
    return text;
}

bool Warhead::File::FindFile(std::string_view findName, std::string const& pathFind, bool recursive /*= false*/)
{
    Poco::Path currPath(pathFind);

    for (auto const& dirEntry : std::filesystem::directory_iterator(currPath.absolute().toString()))
    {
        auto const& path = dirEntry.path();

        if (std::filesystem::is_directory(path) && recursive)
            FindFile(findName, pathFind, true);

        if (path.filename().generic_string() == findName)
            return true;
    }

    return false;
}

bool Warhead::File::FindDirectory(std::string_view findName, std::string const& pathFind, bool recursive /*= false*/)
{
    Poco::Path currPath(pathFind);

    for (auto const& dirEntry : std::filesystem::directory_iterator(currPath.absolute().toString()))
    {
        auto const& path = dirEntry.path();

        if (!std::filesystem::is_directory(path))
            continue;

        if (path.filename() == findName)
            return true;

        if (recursive)
            FindDirectory(findName, pathFind, true);
    }

    return false;
}

void Warhead::File::FillFileList(std::vector<std::string>& pathList, std::string const& pathFill, bool recursive /*= false*/)
{
    Poco::Path currPath(pathFill);

    for (auto const& dirEntry : std::filesystem::directory_iterator(currPath.absolute().toString()))
    {
        auto const& path = dirEntry.path();

        if (std::filesystem::is_directory(path) && recursive)
            FillFileList(pathList, pathFill, true);

        pathList.emplace_back(path.generic_string());
    }
}

std::string Warhead::File::GetFileName(std::string const& filePath)
{
    Poco::Path path(filePath);
    return path.getFileName();
}

uint64 Warhead::File::GetFileSize(std::string const& filePath)
{
    Poco::File file(filePath);
    return file.getSize();
}
