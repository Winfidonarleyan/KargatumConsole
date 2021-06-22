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

#include "Common.h"
#include "Lab.h"
#include "GitRevision.h"
#include "Log.h"
#include "StringConvert.h"
#include "Timer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <random>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include <Poco/Timestamp.h>

namespace fs = std::filesystem;

namespace
{
    using NumbersTemplate = std::vector<int>;

    NumbersTemplate _numbers;
    NumbersTemplate _numbers1;
    NumbersTemplate _numbers2;

    //
    constexpr auto FILE_PATH = "RandomNumbers.txt";
    std::mutex _lock;

    constexpr auto numbersCount = 20000;
    constexpr auto numbersMin = 10;
    constexpr auto numbersMax = 100;
}

inline auto GetTimeDiff(SystemTimePoint timePoint)
{
    using namespace std::chrono;

    // Get End Time
    auto end = system_clock::now();
    return duration_cast<Microseconds>(end - timePoint).count();
}

inline Poco::Timestamp::TimeDiff GetUSTimeDiffToNow(Poco::Timestamp& startTime)
{
    Poco::Timestamp::TimeDiff diff = startTime.elapsed(); // how long did it take?
    Poco::Timestamp start(startTime); // save start time
    startTime.update(); // update with current time
    diff = startTime - start; // again, how long?

    return diff;
}

void GenerateFile()
{
    //LOG_DEBUG("> Start generate file...");

    // Path to file
    fs::path path = fs::path(FILE_PATH);

    std::ofstream file(path);
    if (!file.is_open())
    {
        LOG_FATAL("Failed open file \"%s\"!", path.generic_string().c_str());
        return;
    }

    // Get start time
    Poco::Timestamp startTime;

    //LOG_DEBUG("> File (%s) created", path.filename().generic_string().c_str());
    //LOG_DEBUG("> Start adding numbers. Numbers count (%i). Min/max ({}/{})", numbersCount, numbersMin, numbersMax);

    std::random_device random_device; // Источник энтропии.
    std::mt19937 generator(random_device()); // Генератор случайных чисел.

    // Print numbersCount in file
    file << Warhead::ToString(numbersCount) + "\n";

    for (int i = 0; i < numbersCount; i++)
    {
        std::uniform_int_distribution<> distribution(numbersMin, numbersMax);
        auto number = distribution(generator);
        file << Warhead::ToString(number) + " ";
    }

    file.close();

    LOG_INFO("# -- File created in '{}'", Warhead::Time::ToTimeString<Microseconds>(GetUSTimeDiffToNow(startTime), TimeOutput::Microseconds).c_str());
    //std::string test = Warhead::StringFormat("# -- File created in {}\n", Warhead::Time::ToTimeString<Microseconds>(GetUSTimeDiffToNow(startTime), TimeOutput::Microseconds));

    //fmt::print("> {}", test);
}

void GetNumbers()
{
    auto fileText = Warhead::File::GetFileText(FILE_PATH);

    auto found = fileText.find_first_of('\n');
    if (found != std::string::npos)
        fileText = fileText.substr(found + 2LL);
    else
    {
        LOG_FATAL("> In file ({}) not found array", FILE_PATH);
        return;
    }

    for (auto str : Warhead::Tokenize(fileText, ' ', false))
    {
        auto number = Warhead::StringTo<uint32>(str);
        if (!number)
        {
            LOG_FATAL("> Number ({}) is incorrect!", str);
            continue;
        }

        _numbers.emplace_back(*number);
    }

    //LOG_INFO(" ");
    //LOG_INFO("> Added ({}) nubmers", _numbers.size());

    uint32 count = 0;

    for (auto const& itr : _numbers)
    {
        count++;

        if (count >= numbersCount / 2 + 1)
            break;

        _numbers1.emplace_back(itr);
    }

    for (size_t i = numbersCount / 2; i < _numbers.size(); i++)
    {
        _numbers2.emplace_back(_numbers.at(i));
    }
}

int main()
{
    //GenerateFile();
    //GetNumbers();

    LOG_INFO("{:s}", true);

    return 0;
}
