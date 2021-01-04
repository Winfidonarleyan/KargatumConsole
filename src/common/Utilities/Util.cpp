/*
 * Copyright (C) 
 */

#include "Util.h"
#include "Common.h"

std::vector<std::string_view> Warhead::Tokenize(std::string_view str, char sep, bool keepEmpty)
{
    std::vector<std::string_view> tokens;

    size_t start = 0;
    for (size_t end = str.find(sep); end != std::string_view::npos; end = str.find(sep, start))
    {
        if (keepEmpty || (start < end))
            tokens.push_back(str.substr(start, end - start));

        start = end + 1;
    }

    if (keepEmpty || (start < str.length()))
        tokens.push_back(str.substr(start));

    return tokens;
}

bool StringEqualI(std::string_view a, std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
}

