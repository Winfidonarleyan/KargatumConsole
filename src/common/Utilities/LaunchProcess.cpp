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

#include "LaunchProcess.h"
#include "Log.h"
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/StreamCopier.h>

namespace
{
    // Path to powershell
    constexpr auto POWER_SHELL_EXE = "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";

    // Git commands
    constexpr auto GIT_ADD_BRANCH = "git checkout -b";
    constexpr auto GIT_ADD_FILES = "git add .";
    constexpr auto GIT_COMMIT = "git commit -m";
    constexpr auto GIT_COMMIT_MESSAGE = "\"feat(Misc): correct\"";

    bool IsExitstText(std::string_view text, std::string_view textFind)
    {
        return text.find(textFind) != std::string::npos;
    };
}

void Warhead::Process::SendCommand(std::string_view path, std::string_view command)
{
    std::vector<std::string> args = { { "-command" } };

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, 0);

    std::string text;
    Poco::PipeInputStream istr(outPipe);
    Poco::StreamCopier::copyToString(istr, text);

    LOG_INFO("\n{}", text);
}

void Warhead::Process::Git::CommitAllFiles(std::string_view path, std::string_view commitMessage)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("{}; {} {}", GIT_ADD_FILES, GIT_COMMIT, commitMessage);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, 0);

    std::string text;
    Poco::PipeInputStream istr(outPipe);
    Poco::StreamCopier::copyToString(istr, text);

    LOG_INFO("\n{}", text);
}

void Warhead::Process::Git::CreateBranch(std::string_view path, std::string_view branchName)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("{} {}", GIT_ADD_BRANCH, branchName);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, 0);

    std::string text;
    Poco::PipeInputStream istr(outPipe);
    Poco::StreamCopier::copyToString(istr, text);

    LOG_INFO("\n{}", text);
}

void Warhead::Process::Git::Clone(std::string_view path, std::string_view url)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("git clone {}", url);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, 0);

    std::string text;
    Poco::PipeInputStream istr(outPipe);
    Poco::StreamCopier::copyToString(istr, text);

    LOG_INFO("\n{}", text);
}

bool Warhead::Process::Git::IsCorrectUpstream(std::string_view path, std::string_view upstreamName)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("git remote show {}", upstreamName);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Pipe errPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, &errPipe);

    std::string text;
    std::string textErr;
    Poco::PipeInputStream istr(outPipe);
    Poco::PipeInputStream istr1(errPipe);
    Poco::StreamCopier::copyToString(istr, text);
    Poco::StreamCopier::copyToString(istr1, textErr);    

    if (!textErr.empty() && IsExitstText(textErr, fmt::format("fatal: '{}' does not appear to be a git repository", upstreamName)))
    {
        LOG_FATAL(">> Upstream ({}) is incorrect!", upstreamName);
        return false;
    }

    if (!text.empty() && IsExitstText(text, fmt::format("* remote {}", upstreamName)))
    {
        LOG_DEBUG(">> Upstream ({}) is correct", upstreamName);
        return true;
    }

    LOG_FATAL(">> Upstream ({}) is invalid!", upstreamName);
    return false;
}

void Warhead::Process::Git::CheckoutPR(std::string_view path, std::string_view upstreamName, std::string_view prID)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("git pull {} pull/{}/head", upstreamName, prID);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe errPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, 0, &errPipe);

    std::string textErr;
    Poco::PipeInputStream istr1(errPipe);
    Poco::StreamCopier::copyToString(istr1, textErr);

    // fatal: couldn't find remote ref pull/324234234/head
    if (!textErr.empty() && IsExitstText(textErr, fmt::format("fatal: couldn't find remote ref pull/{}/head", prID)))
        LOG_ERROR("{}", textErr);
}

bool Warhead::Process::Git::IsExistBranch(std::string_view path, std::string_view name)
{
    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("git branch --list {}", name);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Pipe outPipe;
    Poco::Process::launch(POWER_SHELL_EXE, args, 0, &outPipe, 0);

    std::string text;
    Poco::PipeInputStream istr(outPipe);
    Poco::StreamCopier::copyToString(istr, text);

    return !text.empty();
}

void Warhead::Process::Git::Checkout(std::string_view path, std::string_view branchName)
{
    if (!IsExistBranch(path, branchName))
    {
        LOG_ERROR(">> Not exist branch ({})", branchName);
        return;
    }

    std::vector<std::string> args = { { "-command" } };

    std::string command = fmt::format("git checkout {}", branchName);

    // Add commands
    args.emplace_back(StringFormat("cd {}; {}", path, command));

    Poco::Process::launch(POWER_SHELL_EXE, args, 0, 0, 0);
}
