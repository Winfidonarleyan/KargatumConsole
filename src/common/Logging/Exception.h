/*

 */


#ifndef _WARHEAD_EXCEPTION_H_
#define _WARHEAD_EXCEPTION_H_

#include "Define.h"
#include <fmt/format.h>

namespace Warhead
{
    class WH_COMMON_API Exception
    {
    public:
        /// Creates an exception with message.
        Exception(std::string_view msg) : _msg(msg) { }

        /// Creates an exception with fmt message.
        template<typename... Args>
        Exception(std::string_view fmt, Args&&... args)
        {
            try
            {
                _msg = fmt::format(fmt, std::forward<Args>(args)...);
            }
            catch (const fmt::format_error& formatError)
            {
                _msg = fmt::format("An error occurred formatting string \"{}\": {}", fmt, formatError.what());
            }
        }

        /// Destroys the exception.
        ~Exception() noexcept = default;
        
        /// Returns a string consisting of the
        /// message name and the message text.
        inline std::string_view GetErrorMessage() const { return _msg; }
	
    private:
        std::string _msg;

        Exception(Exception const&) = delete;
        Exception(Exception&&) = delete;
        Exception& operator=(Exception const&) = delete;
        Exception& operator=(Exception&&) = delete;
    };

} // namespace Warhead

#endif //
