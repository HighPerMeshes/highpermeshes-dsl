// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_LOOPTYPES_EXECUTION_POLICY_HPP
#define DSL_LOOPTYPES_EXECUTION_POLICY_HPP

#include <type_traits>

namespace HPM
{
    //! The ExecutionPolicy enum defines different types of data processing of the loop.
    enum class ExecutionPolicy
    {
        Scalar,
        SIMD
    };
} // namespace HPM

#endif