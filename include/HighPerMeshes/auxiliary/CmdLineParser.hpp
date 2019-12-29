// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_CMDLINEPARSER_HPP
#define AUXILIARY_CMDLINEPARSER_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

namespace HPM::auxiliary
{
    auto CommandLineReader(int argc, char** argv, const std::string& key, const std::string& hint) -> std::string
    {
        bool key_found = false;
        int index = 1;

        while (index < argc)
        {
            const std::string argument(argv[index]);

            if (!key_found)
            {
                // Parameter names always start with a '-' character.
                if (argument.at(0) != '-')
                {
                    ++index;
                    continue;
                }

                // Get the first character after '-' that is not equal to '-' and test for `key` being a prefix of what is behind '-'.
                // If this is not the case, continue.
                const std::size_t key_begin = argument.find_first_not_of("-", 1);
                if (key_begin == std::string::npos || argument.substr(key_begin, key.size()) != key)
                {
                    ++index;
                    continue;
                }

                // Jump to the character after the requested `key` and test for either end-of-argument or '='.
                // If this is the case, the key has been found. Otherwise, continue.
                const std::size_t key_end = key_begin + key.size();
                if (key_end < argument.size() && argument.at(key_end) != '=')
                {
                    ++index;
                    continue;
                }
                key_found = true;

                // Get the string after the '=' (the '=' character is skipped in any case, even if not present).
                // If this string is not empty, we have found the value. 
                const std::size_t value_begin = argument.find_first_not_of("=", key_end);
                if (value_begin != std::string::npos)
                {
                    return argument.substr(value_begin);
                }
            }
            else
            {
                // We do not expect a paramter name here.
                if (argument.at(0) == '-')
                {
                    std::cerr << "Warning: no value set for parameter " << key << " -- parameter ignored." << std::endl;
                    key_found = false;
                }
                else
                {
                    // Get the string after '=' (if present).
                    // This is the value.
                    const std::size_t value_begin = argument.find_first_not_of("=");
                    if (value_begin != std::string::npos)
                    {
                        return argument.substr(value_begin);
                    }
                }
            }

            ++index;
        }

        std::stringstream error_stream;
        error_stream << "Parameter -" << key << " (=" << hint << ") not found in commandline arguments";
        throw std::runtime_error(error_stream.str());
    }
} // namespace HPM::auxiliary

#endif