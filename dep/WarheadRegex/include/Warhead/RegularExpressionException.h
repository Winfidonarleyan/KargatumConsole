

#ifndef _WARHEAD_RE_EXCEPTION_H_
#define _WARHEAD_RE_EXCEPTION_H_

#include <exception>
#include <string_view>

namespace Warhead
{
    class RegularExpressionException
    {
    public:
        /// Creates an exception with message.
        RegularExpressionException(std::string_view msg) : _msg(msg) { }

        /// Destroys the exception.
        ~RegularExpressionException() noexcept = default;

        /// Returns a string consisting of the message text.
        inline std::string_view GetErrorMessage() const { return _msg; }

    private:
        std::string _msg;

        RegularExpressionException(RegularExpressionException const&) = delete;
        RegularExpressionException(RegularExpressionException&&) = delete;
        RegularExpressionException& operator=(RegularExpressionException const&) = delete;
        RegularExpressionException& operator=(RegularExpressionException&&) = delete;
    };
}

#endif // _WARHEAD_RE_EXCEPTION_H_
