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

//
constexpr auto FILE_PATH = "RandomNumbers.txt";
std::mutex _lock;

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
    LOG_DEBUG("> Start generate file...");

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

    auto numbersCount = 20000;
    auto numbersMin = 1;
    auto numbersMax = 100;

    LOG_DEBUG("> File (%s) created", path.filename().generic_string().c_str());
    LOG_DEBUG("> Start adding numbers. Numbers count (%i). Min/max (%u/%u)", numbersCount, numbersMin, numbersMax);

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

    LOG_INFO("# -- File created in '%s'", Warhead::Time::ToTimeString<Microseconds>(GetUSTimeDiffToNow(startTime), TimeOutput::Microseconds).c_str());
}

void CheckFile()
{
    //std::scoped_lock<std::mutex> guard(_lock);
    std::lock_guard<std::mutex> guard(_lock);

    auto fileText = Warhead::File::GetFileText(FILE_PATH);

    auto found = fileText.find_first_of('\n');
    if (found != std::string::npos)
        fileText = fileText.substr(found + 2LL);
    else
    {
        LOG_FATAL("> In file (%s) not found array", FILE_PATH);
        return;
    }

    std::vector<int> numbers;

    for (auto str : Warhead::Tokenize(fileText, ' ', false))
    {
        auto number = Warhead::StringTo<uint32>(str);
        if (!number)
        {
            LOG_FATAL("> Number (%.*s) is incorrect!", STRING_VIEW_FMT_ARG(str));
            continue;
        }

        //LOG_DEBUG("> Add '%u'", *number);

        numbers.emplace_back(*number);
    }

    LOG_INFO(" ");
    LOG_INFO("> Added (%u) nubmers", numbers.size());

    auto resultMinIndex = std::distance(numbers.begin(), std::min_element(numbers.begin(), numbers.end()));
    int minElement = numbers.at(resultMinIndex);
    int minCount = std::count(numbers.begin(), numbers.end(), minElement);

    LOG_INFO("> Min element at: %i. Count: %i", minElement, minCount);
}

int main()
{
    GenerateFile();

    // Get start time
    Poco::Timestamp startTime;

    CheckFile();

    LOG_INFO("--");
    LOG_INFO("# CheckFile done with 1 thread in '%s'", Warhead::Time::ToTimeString<Microseconds>(GetUSTimeDiffToNow(startTime), TimeOutput::Microseconds).c_str());
    LOG_INFO("--");

    startTime.update();

    std::vector<std::thread> threads;

    threads.emplace_back(CheckFile);
    threads.emplace_back(CheckFile);

    for (auto& thr : threads)
        thr.join();

    LOG_INFO("--");
    LOG_INFO("# CheckFile done with 2 thread in '%s'", Warhead::Time::ToTimeString<Microseconds>(GetUSTimeDiffToNow(startTime), TimeOutput::Microseconds).c_str());
    LOG_INFO("--");

    return 0;
}
