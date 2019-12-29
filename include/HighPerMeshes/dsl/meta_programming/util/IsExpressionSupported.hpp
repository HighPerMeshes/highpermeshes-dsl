// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_METAPROGRAMMING_UTIL_ISEXPRESSIONSUPPORTED_HPP
#define DSL_METAPROGRAMMING_UTIL_ISEXPRESSIONSUPPORTED_HPP

#include <type_traits>

namespace HPM
{
    //!
    //! This template definition takes a list of template parameters.
    //! As the compiler has to evaluate the template arguments this can be used to check whether the template argument
    //! is valid or not.
    //!
    //! \tparam T template arguments expression that the compiler should evaluate. It is disregarded afterwards.
    //!
    template <typename... T>
    using TryToInstantiate = void;

    //!
    //! The only purpose of this typename definition is to make the code below easier to understand.
    //! Instead of disregard_this you could also write void.
    //!
    using DisregardThis = void;

    //!
    //! Base case of is_supported_impl.
    //! \tparam Expression which is check if it is possible to instantiate it
    //! \tparam Attempt placeholder
    //! \tparam T template arguments for Expression
    //!
    template <template <typename...> typename ExpressionT, typename Attempt, typename... T>
    struct IsSupportedImpl : std::false_type
    {
    };

    //!
    //! Special case that is is possible to instantiate Expression
    //! \tparam Expression which is check if it is possible to instantiate it
    //! \tparam T template arguments for Expression
    //!
    template <template <typename...> typename ExpressionT, typename... T>
    struct IsSupportedImpl<ExpressionT, TryToInstantiate<ExpressionT<T...>>, T...> : std::true_type
    {
    };

    //!
    //! This works as an interface for the TMP function 'IsSupportedImpl'.
    //! \tparam Expression which is check if it is possible to instantiate it
    //! \tparam T template arguments for Expression
    //!
    template <template <typename...> typename ExpressionT, typename... T>
    constexpr bool IsSupported = IsSupportedImpl<ExpressionT, DisregardThis, T...>::value;
} // namespace HPM

#endif