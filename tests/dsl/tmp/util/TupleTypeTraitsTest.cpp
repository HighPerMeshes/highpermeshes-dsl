// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>
#include <tuple>
#include <type_traits>

#include <HighPerMeshes/dsl/meta_programming/util/TupleTypeTraits.hpp>

class DummyClass
{
};

TEST(TupelTypeTraitsTest, IsTrueForEachTypeInTuple)
{
    static_assert(HPM::IsTrueForEachTypeInTuple<std::is_integral, std::tuple<int, int>>);
    static_assert(HPM::IsTrueForEachTypeInTuple<std::is_default_constructible, std::tuple<DummyClass, DummyClass>>);
    static_assert(HPM::IsTrueForEachTypeInTuple<std::is_pointer, std::tuple<int*, float*, double*>>);
    static_assert(not HPM::IsTrueForEachTypeInTuple<std::is_integral, std::tuple<int, double, DummyClass>>);
}
