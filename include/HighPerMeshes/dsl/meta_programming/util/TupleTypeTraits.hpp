// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_METAPROGRAMMING_UTIL_TUPLETYPETRAITS_HPP
#define DSL_METAPROGRAMMING_UTIL_TUPLETYPETRAITS_HPP

#include <tuple>
#include <type_traits>

namespace HPM
{
    //!
    //! Base case: TupleT is not a std::tuple -> return false
    //!
    template <template <typename> typename Expression, typename TupleT>
    struct IsTrueForEachTypeInTupleImplementation : std::false_type
    {
    };

    //!
    //! Special case: The given type is a tuple -> check all extracted types
    //!
    template <template <typename> typename Expression, typename... T>
    struct IsTrueForEachTypeInTupleImplementation<Expression, std::tuple<T...>>
    {
        static constexpr bool value{std::conjunction_v<Expression<T>...>};
    };

    //!
    //! Checks whether the given Expression is true (std::true_type) for all types in the std::tuple TupleT.
    //! \tparam Expression that is evaluated for each type in TupleT.
    //! Return value should be stored in public member 'value' (std::true_type or std::false_type).
    //! \tparam TupleT type to check
    //!
    template <template <typename> typename Expression, typename TupleT>
    constexpr bool IsTrueForEachTypeInTuple = IsTrueForEachTypeInTupleImplementation<Expression, TupleT>::value;
} // namespace HPM

#endif