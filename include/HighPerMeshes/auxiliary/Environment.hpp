// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_ENVIRONMENT_HPP
#define AUXILIARY_ENVIRONMENT_HPP

#include <cstdint>
#include <cstdlib>
#include <string>
#include <type_traits>

namespace HPM::auxiliary
{
    //!
    //! \brief Get an enviroment variable.
    //!
    //! This function returns the value assigned to an environment variable.
    //! If the environment variable cannot be found or is empty, a user-defined default value is returned.
    //!
    //! Usage:
    //!
    //! in your shell (here bash)
    //! ```
    //!     $ export Env_1=3
    //!     $ export Env_2=1.3
    //! ```
    //! in your program
    //! ```
    //!     int value = GetEnv<int>("Env_1", 12);    -> value is 3
    //!     auto value = GetEnv("Env_3", 12);        -> value is 12
    //!     auto value = GetEnv("Env_2", 3.0f);      -> value is 1.3f
    //! ```
    //!  
    //! \tparam T the type of the value: integers and floating point numbers are supported
    //! \param name the name of the environment variable
    //! \param default_value this value is returned if the environment variable cannot be found or is empty
    //! \return either the value assigned to the specified environment variable or the default value 
    //!
    template <typename T>
    auto GetEnv(const std::string& name, const T default_value)
    {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "error: only integers and floating point numbers are supported");

        if (const char* value = std::getenv(name.c_str()))
        {
            if constexpr (std::is_integral_v<T>)
            {
                return std::atoi(value);
            }
            else
            {
                return std::atof(value);
            }
        }
        
        return default_value;
    }
} // namespace HPM::auxiliary

#endif