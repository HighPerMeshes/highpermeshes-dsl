// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_CONSTEXPRFOR_HPP
#define AUXILIARY_CONSTEXPRFOR_HPP

#include <type_traits>
#include <utility>

#include <HighPerMeshes/common/DataTypes.hpp>

namespace HPM::auxiliary
{
    namespace
    {
        using ::HPM::dataType::ConstexprArray;
        
        //!
        //! \brief Compile time loop implementation.
        //!
        //! Equivalent to:
        //! ```
        //!   for (std::size_t i = Begin; i < End; i += Increment) { loop_body(i); }
        //! ```
        //! The loop body (`loop_body`) must be a callable of the following form:
        //! ```
        //!   type (*) (const auto&)
        //! ```
        //! e.g., a Lambda like this:
        //! ```
        //!   [..] (const auto& I) { .. // loop index is I }
        //! ```
        //!
        //! \tparam Begin start index
        //! \tparam Increment loop index increment
        //! \tparam I parameter pack holding a sequence of integers starting from 0 to the number of loop iterations
        //! \tparam LoopBodyT the type of the callable (loop body) to be executed
        //! \param loop_body a callable that implements the body of the loop
        //! \param unnamed (not used) an index-sequence type for template parameter deduction
        //! \return depending on the return type of `loop_body` either `void` or a `ConstexprArray` type holding the return values of the individual loop body executions
        //!
        template <std::size_t Begin, std::size_t Increment, typename LoopBodyT, std::size_t... I>
        constexpr auto ConstexprFor(LoopBodyT loop_body, std::index_sequence<I...>)
        {
            // Get the return type of the callable.
            static_assert(std::conjunction_v<std::is_invocable<LoopBodyT, std::integral_constant<std::size_t, Begin + I * Increment>>...>, "error: callable must have this type 'type (*) (const auto&)'");
            using LoopBodyTReturnT = decltype(loop_body(std::integral_constant<std::size_t, 0>{}));

            // If the callable returns 'void', use fold expression.
            if constexpr (std::is_void_v<LoopBodyTReturnT>)
            {
                (loop_body(std::integral_constant<std::size_t, Begin + I * Increment>{}), ...);
            }
            // Otherwise, create a 'ConstexprArray' of appropriate type: values returned by the callable are passed as template parameters.
            else
            {
                return ConstexprArray<LoopBodyTReturnT, loop_body(std::integral_constant<std::size_t, Begin + I * Increment>{})...>{};
            }
        }
    } // namespace

    //!
    //! \brief Compile time loop wrapper.
    //!
    //! The general case: loop index begin, end, and increment are provided.
    //!
    //! Equivalent to:
    //! ```
    //!   for (std::size_t i = Begin; i < End; i += Increment) { loop_body(i); }
    //! ```
    //!
    //! \tparam Begin start index
    //! \tparam End end index
    //! \tparam Increment loop index increment
    //! \tparam LoopBodyT the type of the callable (loop body) to be executed
    //! \param loop_body a callable that implements the body of the loop
    //! \return depending on the return type of `loop_body` either `void` or a `ConstexprArray` type holding the return values of the individual loop body executions
    //!
    template <std::size_t Begin, std::size_t End, std::size_t Increment, typename LoopBodyT>
    constexpr auto ConstexprFor(LoopBodyT loop_body)
    {
        static_assert(Begin <= End && Increment > 0, "error: this loop type is not supported");

        constexpr std::size_t IterationCount = (End - Begin + Increment - 1) / Increment;

        return ConstexprFor<Begin, Increment>(loop_body, std::make_index_sequence<IterationCount>{});
    }

    //!
    //! \brief Compile time loop wrapper.
    //!
    //! The general case: loop index begin and end are provided.
    //!
    //! Equivalent to:
    //! ```
    //!   for (std::size_t i = Begin; i < End; ++i) { loop_body(i); }
    //! ```
    //!
    //! \tparam Begin start index
    //! \tparam End end index
    //! \tparam LoopBodyT the type of the callable (loop body) to be executed
    //! \param loop_body a callable that implements the body of the loop
    //! \return depending on the return type of `loop_body` either `void` or a `ConstexprArray` type holding the return values of the individual loop body executions
    //!
    template <std::size_t Begin, std::size_t End, typename LoopBodyT>
    constexpr auto ConstexprFor(LoopBodyT loop_body)
    {
        return ConstexprFor<Begin, End, 1>(loop_body);
    }

    //!
    //! \brief Compile time loop wrapper.
    //!
    //! The general case: loop index end is provided.
    //!
    //! Equivalent to:
    //! ```
    //!   for (std::size_t i = 0; i < End; ++i) { loop_body(i); }
    //! ```
    //!
    //! \tparam End end index
    //! \tparam LoopBodyT the type of the callable (loop body) to be executed
    //! \param loop_body a callable that implements the body of the loop
    //! \return depending on the return type of `loop_body` either `void` or a `ConstexprArray` type holding the return values of the individual loop body executions
    //!
    template <std::size_t End, typename LoopBodyT>
    constexpr auto ConstexprFor(LoopBodyT loop_body)
    {
        return ConstexprFor<0, End, 1>(loop_body);
    }
} // namespace HPM::auxiliary

#endif