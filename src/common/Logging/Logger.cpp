/*

*/

#include "Logger.h"

void Warhead::Logger::AddChannel(std::shared_ptr<Channel> channel)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _channels.emplace_back(channel);
}

void Warhead::Logger::Write(LogMessage const& msg)
{
    std::lock_guard<std::mutex> guard(_mutex);

    if (_level < msg.GetLevel())
        return;

    for (auto const& channel : _channels)
    {
        if (channel->GetLevel() < msg.GetLevel())
            continue;

        channel->Write(msg);
    }
}

void Warhead::Logger::DeleteChannel(std::string_view name)
{
    if (_channels.empty())
        return;

    std::lock_guard<std::mutex> guard(_mutex);

    for (auto const& channel : _channels)
    {
        if (channel->GetName() == name)
        {
            std::erase(_channels, channel);
            return;
        }
    }
}
