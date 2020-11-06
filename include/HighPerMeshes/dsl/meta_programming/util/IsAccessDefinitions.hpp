// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_METAPROGRAMMING_UTIL_ISACCESSDEFINITIONS_HPP
#define DSL_METAPROGRAMMING_UTIL_ISACCESSDEFINITIONS_HPP

#include <tuple>
#include <type_traits>

#include <HighPerMeshes/dsl/data_access/AccessDefinition.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsTemplateSpecialization.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/TupleTypeTraits.hpp>

namespace HPM
{
    //! Base case: T is not a AccessDefinition
    template <typename T>
    struct IsAccessDefinitionImpl : std::false_type
    {
    };

    //! Special case: T is a AccessDefinition
    template <typename BufferT, typename AccessPatternT, size_t RequestedDim, AccessMode DataAccess>
    struct IsAccessDefinitionImpl<AccessDefinition<BufferT, AccessPatternT, RequestedDim, DataAccess>> : std::true_type
    {
    };

    //! Checks whether the type T is a AccessDefinition or not.
    template <typename T>
    constexpr bool IsAccessDefinition = IsAccessDefinitionImpl<T>::value;

    //! Checks if T is a valid DataAccesses type (std::tuple of AccessDefinitions).
    template <typename T>
    struct IsAccessDefinitionsImpl
    {
        static constexpr bool value = {IsTemplateSpecialization<T, std::tuple> && IsTrueForEachTypeInTuple<IsAccessDefinitionImpl, T>};
    };

    //! Checks if T is a valid DataAccesses type (std::tuple of AccessDefinitions).
    template <typename T>
    constexpr bool IsAccessDefinitions = IsAccessDefinitionsImpl<T>::value;
} // namespace HPM

#endif