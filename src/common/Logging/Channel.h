/*

 */


#ifndef _WARHEAD_CHHANEL_H_
#define _WARHEAD_CHHANEL_H_

#include "Define.h"
#include "LogCommon.h"
#include <memory>
#include <string_view>
#include <vector>

namespace Warhead
{
    class LogMessage;

    class WH_COMMON_API Channel
    {
    public:
        Channel(ChannelType type, std::string_view name, LogLevel level, std::string_view pattern);
        virtual ~Channel() = default;

        inline std::string_view GetName() { return _name; }
        inline LogLevel GetLevel() { return _level; }
        inline void SetLevel(LogLevel const level) { _level = level; }
        inline ChannelType GetType() { return _type; }

        void SetPattern(std::string_view pattern);
        void Format(LogMessage const& msg, std::string& text);

        virtual void Write(LogMessage const& msg) = 0;
        virtual void SetRealmId(uint32 /*realmId*/) { }

    private:
        struct PatternAction
        {
            PatternAction() = default;

            char key{ 0 };
            std::size_t length{ 0 };
            std::string property;
            std::string prepend;
        };

        void ParsePattern();

        ChannelType _type{ ChannelType::Console };
        std::string _name;
        LogLevel _level{ LogLevel::Disabled };

        std::string _pattern;
        std::vector<PatternAction> _patternActions;

        Channel(const Channel&) = delete;
	    Channel& operator= (const Channel&) = delete;
    };

} // namespace Warhead

#endif //
