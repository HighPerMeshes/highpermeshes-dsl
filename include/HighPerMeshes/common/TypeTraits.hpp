// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_TYPESTRAITS_HPP
#define COMMON_TYPESTRAITS_HPP

namespace HPM::dataType::internal
{
    template <typename T>
    struct ProvidesMetaData
    {
        static constexpr bool value = false;
    };
} // namespace HPM::dataType::internal

#endif