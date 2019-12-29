// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_ACCESSMODE_HPP
#define DSL_DATAACCESS_ACCESSMODE_HPP

#include <type_traits>

namespace HPM
{
    //! The AccessMode enum defines different types of data accesses during iteration over the grid.
    enum class AccessMode
    {
        Read,
        Write,
        ReadWrite,
        Accumulate // \todo { Accumulate needed? - Stefan G. 2.12.19 }
    };

    //! Compile-time constants for the AccessMode enum which can be used to pass them to different functions.
    //! \{
    constexpr std::integral_constant<AccessMode, AccessMode::Read> ReadConstant{};
    constexpr std::integral_constant<AccessMode, AccessMode::Write> WriteConstant{};
    constexpr std::integral_constant<AccessMode, AccessMode::ReadWrite> ReadWriteConstant{};
    constexpr std::integral_constant<AccessMode, AccessMode::Accumulate> AccumulateConstant{};
    //! \}
} // namespace HPM

#endif