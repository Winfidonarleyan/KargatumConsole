/*

 */


#ifndef _WARHEAD_FILE_CHANNEL_H_
#define _WARHEAD_FILE_CHANNEL_H_

#include "Channel.h"
#include <mutex>

namespace Warhead
{
    class WH_COMMON_API FileChannel : public Channel
    {
    public:
        static constexpr auto ThisChannelType{ ChannelType::File };

        FileChannel(std::string_view name, LogLevel level, std::string_view pattern, std::vector<std::string_view> const& options);
        virtual ~FileChannel();

        void Write(LogMessage const& msg) override;

    private:
        bool OpenFile();
        void CloseFile();

        std::string _logsDir;
        std::string _fileName;
        std::unique_ptr<std::ofstream> _logFile;
        bool _isDynamicFileName{ false };
        bool _isFlush{ true };
        bool _isOpenModeAppend{ true };
        bool _isAddTimestamp{ false };
        std::mutex _mutex;
    };

} // namespace Warhead

#endif //
