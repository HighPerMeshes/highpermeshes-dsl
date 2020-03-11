// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_CONFIGPARSER_HPP
#define AUXILIARY_CONFIGPARSER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

namespace HPM::auxiliary
{
    namespace
    {
        //!
        //! \brief A function that posts a message on the console and aborts the execution of the program.
        //!
        void Cerr(const std::string& err)
        {
            std::cout << err;
            std::cin.ignore();
            std::cin.get();

            exit(EXIT_FAILURE);
        }

        //!
        //! \brief A wrapper class which contain function for the conversion of std::string to primitive types (int, float, double, etc.,).
        //!
        struct Conversion
        {
            template <typename T>
            static auto StringToScalar(const std::string& s_value) -> T
            {
                std::istringstream is{s_value};
                T value{};

                if (!(is >> value))
                {
                    Cerr("CFG: invalid " + std::string(typeid(T).name()) + "\n");
                }

                return value;
            }
        };

        template <>
        [[maybe_unused]]
        auto Conversion::StringToScalar<std::string>(const std::string& s_value) -> std::string
        {
            return s_value;
        }
    } // namespace

    //!
    //! \brief A wrapper class which contains functions for parsing user-defined configuration files (.cfg).
    //!
    class ConfigParser
    {
        //!
        //! \brief A function that removes everything from the semicolon (including it) to the end of the line.
        //!
        void RemoveComment(std::string& line) const
        {
            if (line.find(';') != line.npos)
            {
                line.erase(line.find(';'));
            }
        }

        //!
        //! \brief A function that returns false if a non-space character was found, true otherwise. The function is "const" because it does not alter any class member variables.
        //!
        auto IsEmptyLine(const std::string& line) const { return (line.find_first_not_of(' ') == line.npos); }

        //!
        //! \brief A function that extracts the key from the pair of key = value
        //!
        void ExtractKey(std::string& key, const std::size_t& pos_separator, const std::string& line) const
        {
            key = line.substr(0, pos_separator);

            if (key.find('\t') != line.npos || key.find(' ') != line.npos)
            {
                key.erase(key.find_first_of("\t "));
            }
        }

        //!
        //! \brief A function that extracts the value from the pair of key = value.
        //!
        void ExtractValue(std::string& value, const std::size_t pos_separator, const std::string& line) const
        {
            // Create a substring starting from positon of '=' + 1, to the end of the line.
            value = line.substr(pos_separator + 1);
            // Remove the leading IsEmptyLine.
            value.erase(0, value.find_first_not_of("\t "));
            // Remove everything starting with the position of the last non-space character.
            value.erase(value.find_last_not_of("\t ") + 1);
        }

        //!
        //! \brief A function that parse the line by calling above mentioed functions.
        //!
        void ParseLine(const std::string& line, const std::size_t)
        {
            std::string tmp{line};
            tmp.erase(0, tmp.find_first_not_of("\t "));
            const std::size_t pos_separator = tmp.find('=');
            std::string key, value;

            ExtractKey(key, pos_separator, tmp);
            ExtractValue(value, pos_separator, tmp);

            // Check if a key given as parameter already exists in the std::map (data).
            if (!KeyFinder(key))
            {
                data.insert(std::pair<std::string, std::string>(key, value));
            }
            else
            {
                Cerr("CFG: only unique key names are allowed!\n");
            }
        }

        public:
        //!
        //! \brief Constructor.
        //!
        //! Set the name of the configuration file and extract and parse the data.
        //!
        ConfigParser(const std::string& filename) : filename(filename)
        {
            std::ifstream file;
            file.open(filename.c_str());
            if (!file)
            {
                Cerr("CFG: file " + filename + " not found!\n");
            }

            std::string line;
            std::size_t line_number = 0;

            while (std::getline(file, line))
            {
                ++line_number;

                if (line.empty())
                {
                    continue;
                }

                std::string tmp{line};
                RemoveComment(tmp);

                if (IsEmptyLine(tmp))
                {
                    continue;
                }

                ParseLine(tmp, line_number);
            }

            file.close();
        }

        //!
        //! \brief A function for finding the key.
        //!
        auto KeyFinder(const std::string& key) const -> bool { return data.find(key) != data.end(); }

        //!
        //! \brief A function that retrieves the value of a specific key
        //!
        template <typename T>
        auto GetValue(const std::string& key, const T& default_value = T{}) const -> T
        {
            if (!KeyFinder(key))
            {
                return default_value;
            }

            return Conversion::StringToScalar<T>(data.find(key)->second);
        }

        private:
        // Which will hold pairs of key-value
        std::map<std::string, std::string> data;
        // As member variables, we will only have a std::string, which will hold the name of the configuration file
        std::string filename;
    };
} // namespace HPM::auxiliary
#endif