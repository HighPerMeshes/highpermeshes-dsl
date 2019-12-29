// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_TUPLEOPERATIONS_HPP
#define AUXILIARY_TUPLEOPERATIONS_HPP

#include <cstdint>
#include <tuple>
#include <utility>

namespace HPM::auxiliary
{
    //!
    //! \brief Applies an operation to each tuple element.
    //!
    //! Given a tuple `tuple` this functions applies `func` to each element.
    //!
    //! \tparam TupleT a tuple type
    //! \tparam FuncT a callable type
    //! \param tuple a tuple argument
    //! \param func a callable
    //!
    template <typename TupleT, typename FuncT>
    constexpr void TransformTuple(TupleT&& tuple, FuncT func)
    {
        std::apply([&func](auto&&... entries) { (func(std::forward<decltype(entries)>(entries)), ...); }, std::forward<TupleT>(tuple));
    }

    namespace
    {
        template <std::size_t Index, typename FuncT, typename TupleT, typename... T>
        constexpr void TransformTupleIndexed(FuncT&& func, TupleT&& tuple)
        {
            func(std::forward<TupleT>(tuple), std::integral_constant<std::size_t, Index>{});
        }

        template <std::size_t Index, typename FuncT, typename Head, typename... Tail>
        constexpr void TransformTupleIndexed(FuncT&& func, Head&& head, Tail&&... tail)
        {
            func(std::forward<Head>(head), std::integral_constant<std::size_t, Index>{});
            TransformTupleIndexed<Index + 1>(std::forward<FuncT>(func), std::forward<Tail>(tail)...);
        }
    } // namespace

    //!
    //! \brief Applies an operation to each tuple element.
    //!
    //! Given a tuple `tuple` this functions executes `func` to each element where the 1st argument passed to `func`
    //! is the considered element in `tuple` and the 2nd argument passed to `func` is its index as an integral constant.
    //!
    //! \tparam TupleT a tuple type
    //! \tparam FuncT a callable type
    //! \param tuple a tuple argument
    //! \param func a callable
    //!
    template <typename TupleT, typename FuncT>
    constexpr void TransformTupleIndexed(TupleT&& tuple, FuncT&& func)
    {
        std::apply([&func](auto&&... entries) { TransformTupleIndexed<0>(std::forward<FuncT>(func), std::forward<decltype(entries)>(entries)...); }, std::forward<TupleT>(tuple));
    }
} // namespace HPM::auxiliary

#endif