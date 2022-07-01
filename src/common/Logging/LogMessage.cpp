/*
*/

#include "LogMessage.h"

Warhead::LogMessage::LogMessage(std::string_view source, std::string_view text, MessageLevel level, std::string_view option /*= {}*/) :
	_source(source),
	_text(text),
	_level(level),
    _option(option)
{
}

Warhead::LogMessage::LogMessage(std::string_view source, std::string_view text, MessageLevel level,
    std::string_view file, std::size_t line, std::string_view function, std::string_view option /*= {}*/) :
    _source(source),
	_text(text),
    _level(level),
	_file(file),
	_line(line),
    _function(function),
    _option(option)
{
}
