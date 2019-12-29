// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_INDEXSEQUENCE_HPP
#define COMMON_INDEXSEQUENCE_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

namespace HPM::dataType
{
    namespace
    {
        //!
        //! \brief An identiy mapping type.
        //!
        struct Identity
        {
            //!
            //! \brief Identity.
            //!
            //! \tparam Value the value of the integral constant
            //! \param Value an integral constant
            //! \return the value of the integral constant
            //!
            template <std::size_t Value>
            constexpr auto operator()(const std::integral_constant<std::size_t, Value>&)
            {
                return Value;
            }
        };

        //!
        //! \brief Apply a callable (transformation) to each element of an index sequence.
        //!
        //! \tparam TrafoT the type of the callable (transformation)
        //! \tparam I an index parameter pack
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return an index sequence with values given by applying the callable to the parameter pack `I`
        //!
        template <typename TrafoT, std::size_t... I>
        constexpr auto TransformIndexSequence(std::index_sequence<I...>)
        {
            static_assert(std::is_invocable<TrafoT, std::integral_constant<std::size_t, 0>>::value, "error: callable must take an integral constant as argument");

            return std::index_sequence<TrafoT{}(std::integral_constant<std::size_t, I>{})...>{};
        }
    } // namespace

    //!
    //! \brief An index sequence type.
    //!
    //! The definition uses a contiguous index sequence starting from zero (\f$0,1,2,3,..,N-1\f$)
    //! and then applies either the identity mapping or a user-provided transformation to each index.
    //!
    //! \tparam N the length of the index sequence
    //! \tparam TrafoT the type of the callable (transformation)
    //!
    template <std::size_t N, typename TrafoT = Identity>
    using IndexSequence = decltype(TransformIndexSequence<TrafoT>(std::make_index_sequence<N>{}));
} // namespace HPM::dataType

#endif