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

#include "ClientCheck.h"
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include "Util.h"
#include "CryptoHash.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/DirectoryIterator.h>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <list>

namespace
{
    Poco::Path currPath;
    using PathList = std::vector<std::string>;

    std::unordered_map<std::string, uint64> filesSize =
    {
        { "common-2.MPQ", 1756781838 },
        { "common.MPQ", 2856083241 },
        { "expansion.MPQ", 1899508519 },
        { "lichking.MPQ", 2553955175 },
        { "patch-2.MPQ", 1403129115 },
        { "patch-3.MPQ", 605089137 },
        { "patch-3.MPQ", 605089137 },
        { "patch.MPQ", 4005207470 },
        { "backup-ruRU.MPQ", 161091881 },
        { "base-ruRU.MPQ", 22975058 },
        { "expansion-locale-ruRU.MPQ", 16919739 },
        { "expansion-speech-ruRU.MPQ", 269445963 },
        { "lichking-locale-ruRU.MPQ", 12354378 },
        { "lichking-speech-ruRU.MPQ", 6772364 },
        { "locale-ruRU.MPQ", 190129109 },
        { "patch-ruRU-2.MPQ", 270846439 },
        { "patch-ruRU-3.MPQ", 109883630 },
        { "patch-ruRU.MPQ", 614385571 },
        { "speech-ruRU.MPQ", 445791052 }
    };

    std::unordered_map<std::string, uint64> filesSizeCustom =
    {
        { "Patch-7.mpq", 1056336772 },
        { "patch-8.mpq", 510392072 },
        { "patch-a.MPQ", 497229602 },
        { "PATCH-B.MPQ", 1579939796 },
        { "patch-ruRU-4.mpq", 33809711 },
        { "patch-ruRU-5.mpq", 63337162 },
    };

    std::unordered_map<std::string, std::string> filesHash =
    {
        { "backup-ruRU.MPQ", "6bf69de589a40f65da53fd98798ec5ad" },
        { "base-ruRU.MPQ", "3d4da9a3ab3e969717f9630165a65cd4" },
        { "expansion-locale-ruRU.MPQ", "e9ed46fa4e3c5aec30f87f9001fb644a" },
        { "expansion-speech-ruRU.MPQ", "19dabf92cc7aa0f04a9441ff35984116" },
        { "lichking-locale-ruRU.MPQ", "af01bc97ca87104c589d4d84684d3b4e" },
        { "lichking-speech-ruRU.MPQ", "c29bc1a55275d676dcc8923d89abfa19" },
        { "locale-ruRU.MPQ", "b8c8b30b361d9e8850bd878eebbff910" },
        { "patch-ruRU-2.MPQ", "1651f197f2bafdf5b3d9f9004eee64f1" },
        { "patch-ruRU-3.MPQ", "c56223d3af203a27a040e978ce2e313f" },
        { "speech-ruRU.MPQ", "5f2c46d1ed57117d7572e194a20ab7e9" }
    };

    std::vector<std::string> correctPostfix = { "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "a", "s", "d", "f", "g", "h", "j", "k", "l", "z", "x", "c", "v", "b", "n", "m" };

    bool IsSameFileSize(std::string const& fileName, uint64 fileSize)
    {
        auto const& itr = filesSize.find(fileName);
        if (itr == filesSize.end())
        {
            LOG_FATAL("> File '{}' not found!", fileName.c_str());
            return false;
        }

        return fileSize == itr->second;
    }

    bool IsSameWowkaFileSize(std::string const& fileName, uint64 fileSize)
    {
        auto const& itr = filesSizeCustom.find(fileName);
        if (itr == filesSizeCustom.end())
        {
            LOG_FATAL("> File '{}' not found!", fileName.c_str());
            return false;
        }

        return fileSize == itr->second;
    }

    bool IsSameFileHash(std::string const& fileName, std::string const& fileHash)
    {
        auto const& itr = filesHash.find(fileName);
        if (itr == filesHash.end())
        {
            LOG_FATAL("> File '{}' not found!", fileName.c_str());
            return false;
        }

        return fileHash == itr->second;
    }

    bool IsDefaultFile(std::string const& fileName)
    {
        auto const& itr = filesSize.find(Warhead::File::GetFileName(fileName));
        if (itr != filesSize.end())
            return true;

        return false;
    }

    bool IsNormalDirectory()
    {
        return Warhead::File::FindFile("Wow.exe", currPath.absolute().toString());
    }

    bool IsCorrectPostfix(std::string const& postfix)
    {
        for (auto const& itr : correctPostfix)
            if (StringEqualI(itr, postfix))
                return true;

        return false;
    }

    bool IsAppliedClientFile(std::string const& fileName)
    {
        std::string startName;

        size_t found = fileName.find_first_of('-');
        if (found != std::string::npos)
        {
            startName = fileName.substr(0, found);

            if (!StringEqualI(startName, "patch"))
                return false;
        }

        found = fileName.find_last_of('-');
        if (found != std::string::npos)
        {
            std::string name = fileName;
            name.erase(name.begin(), name.begin() + fileName.substr(0, found).size() + 1);
            name.erase(name.end() - 4, name.end());

            if (!IsCorrectPostfix(name))
                return false;
        }

        return true;
    }

    void GetAllFiles(PathList& listFiles)
    {
        PathList pathListAll;

        Warhead::File::FillFileList(pathListAll, currPath.absolute().toString() + "/Data");
        Warhead::File::FillFileList(pathListAll, currPath.absolute().toString() + "/Data/ruRU");

        for (auto const& fileName : pathListAll)
        {
            Poco::Path path(fileName);

            std::string fileExtension = path.getExtension();

            if (StringEqualI(path.getExtension(), "mpq"))
                listFiles.emplace_back(path.toString());
        }
    }

    void GetDefaultFiles(PathList& listFiles)
    {
        PathList pathListAll;

        GetAllFiles(pathListAll);

        for (auto const& fileName : pathListAll)
        {
            if (!IsDefaultFile(Warhead::File::GetFileName(fileName)))
                continue;

            listFiles.emplace_back(fileName);
        }
    }

    void GetNonDefaultFiles(PathList& listFiles)
    {
        PathList pathListAll;

        GetAllFiles(pathListAll);

        for (auto const& fileName : pathListAll)
        {
            if (IsDefaultFile(Warhead::File::GetFileName(fileName)))
                continue;

            listFiles.emplace_back(fileName);
        }
    }

    void CheckDefaultFiles()
    {
        PathList listFiles;

        GetDefaultFiles(listFiles);

        uint32 count = 0;

        for (auto const& path : listFiles)
        {
            std::string fileName = Warhead::File::GetFileName(path);
            uint64 fileSize = Warhead::File::GetFileSize(path);

            if (!IsSameFileSize(fileName, fileSize))
            {
                LOG_INFO("> File '{}' - Modifed", fileName.c_str());
                count++;
            }

            /*if (fileSize > 100000000)
                LOG_INFO("> File '{}' - {}", fileName.c_str(),  ? "Correct" : "Incorrect");
            else
            {
                auto const& hash = Warhead::Crypto::GetMD5HashFromFile(path);

                LOG_INFO("> File '{}' - {}", fileName.c_str(), IsSameFileHash(fileName, hash) ? "Correct" : "Incorrect");
            }*/
        }

        if (!count)
            LOG_INFO("> Very good, all based files correct!");

        LOG_INFO("");
    }

    void CheckNonDefaultFiles()
    {
        PathList listFiles;

        GetNonDefaultFiles(listFiles);

        if (listFiles.empty())
            LOG_INFO("> Not found custom files, very good");

        for (auto const& path : listFiles)
        {
            auto const& fileName = Warhead::File::GetFileName(path);
            auto const& fileSize = Warhead::File::GetFileSize(path);

            if (IsAppliedClientFile(fileName))
                LOG_INFO("> Custom file '{}' - '{}' using by client", fileName.c_str(), fileSize);
            else
                LOG_INFO("> Custom file '{}' - '{}' useless. Client not used this file, need delete'", fileName.c_str(), fileSize);
        }

        LOG_INFO("");
    }

    void CheckWowkaDefaultFiles()
    {
        PathList listFiles;

        GetNonDefaultFiles(listFiles);

        if (listFiles.empty())
            LOG_INFO("> Not found custom files, hmm, clean client?");

        uint32 count = 0;

        for (auto const& path : listFiles)
        {
            auto const& fileName = Warhead::File::GetFileName(path);
            auto const& fileSize = Warhead::File::GetFileSize(path);

            if (!IsSameWowkaFileSize(fileName, fileSize))
            {
                LOG_INFO("> File '{}' - Modifed or old", fileName.c_str());
                count++;
            }
        }

        if (!count)
            LOG_INFO("> Very good, all Wowka.su files correct!");

        LOG_INFO("");
    }
}

ClientCheck* ClientCheck::instance()
{
    static ClientCheck instance;
    return &instance;
}

void ClientCheck::Start()
{
    LOG_INFO("> Using path '{}'", currPath.absolute().toString().c_str());

    if (!IsNormalDirectory())
    {
        LOG_FATAL("> File 'Wow.exe' not found. Directory incorrect");
        system("pause");
        return;
    }

    LOG_INFO("");

    LOG_INFO(">>> Check custom files...");
    CheckNonDefaultFiles();

    LOG_INFO(">>> Check default files...");
    CheckDefaultFiles();

    LOG_INFO(">>> Check Wowka.su files...");
    CheckWowkaDefaultFiles();

    system("pause");
}
