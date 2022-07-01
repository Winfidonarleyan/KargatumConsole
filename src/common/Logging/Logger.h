/*

 */


#ifndef _WARHEAD_LOGGER_H_
#define _WARHEAD_LOGGER_H_

#include "Define.h"
#include "Channel.h"
#include "LogMessage.h"
#include <vector>
#include <mutex>
#include <fmt/core.h>

namespace Warhead
{
    class WH_COMMON_API Logger
    {
    public:
        Logger(std::string_view name, LogLevel level) :
            _name(name), _level(level) { }

        ~Logger() = default;

        inline const std::string_view GetName() const { return _name; }

        void AddChannel(std::shared_ptr<Channel> channel);

        inline void SetLevel(LogLevel level) { _level = level; }
        inline LogLevel GetLevel() const { return _level; }

        void Write(LogMessage const& msg);

        void DeleteChannel(std::string_view name);
        inline void Shutdown() { _channels.clear(); }

    private:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::string _name;
        std::vector<std::shared_ptr<Channel>> _channels;
        LogLevel _level{ LogLevel::Disabled };

        std::mutex _mutex;
    };
}

#endif
