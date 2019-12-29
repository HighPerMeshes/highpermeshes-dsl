// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_METAPROGRAMMING_UTIL_ISTEMPLATESPECIALIZATION_HPP
#define DSL_METAPROGRAMMING_UTIL_ISTEMPLATESPECIALIZATION_HPP

#include <type_traits>

namespace HPM
{
    namespace
    {
        template <typename Specialization, template <typename...> typename NotBaseT>
        struct IsTemplateSpecializationImplementation : std::false_type
        {
        };

        //!
        //! IsTemplateSpecializationImplementation only inherits from true_type if its first template Argument is the same type as the second specialized by any arguments T
        //!
        template <template <typename...> typename BaseT, typename... T>
        struct IsTemplateSpecializationImplementation<BaseT<T...>, BaseT> : std::true_type
        {
        };
    } // namespace

    //!
    //! This template variable determines wether its first template argument `Specialization` is a template specialization of its second template arguement `MaybeBaseT`.
    //!
    //! \tparam Specialization A type specialized by any amount of template typename arguments
    //! \tparam MaybeBaseT A type that normally takes template arguments
    //!
    template <typename Specialization, template <typename...> typename MaybeBaseT>
    constexpr bool IsTemplateSpecialization = IsTemplateSpecializationImplementation<std::decay_t<Specialization>, MaybeBaseT>::value;
} // namespace HPM

#endif