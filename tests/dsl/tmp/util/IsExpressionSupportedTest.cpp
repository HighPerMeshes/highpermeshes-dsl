// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>
#include <tuple>

#include <HighPerMeshes/dsl/meta_programming/util/IsExpressionSupported.hpp>

template <typename A, typename B>
using AssignExpression = decltype(std::declval<A&>() = (std::declval<B&>()));

TEST(IsExpressionSupported, is_supported)
{
    static_assert(HPM::IsSupported<AssignExpression, int, int>);
    static_assert(HPM::IsSupported<AssignExpression, int, char>);
    static_assert(HPM::IsSupported<AssignExpression, std::string, std::string>);
    static_assert(not HPM::IsSupported<AssignExpression, std::string, std::tuple<int, int>>);
    static_assert(not HPM::IsSupported<AssignExpression, int, std::string>);
}
