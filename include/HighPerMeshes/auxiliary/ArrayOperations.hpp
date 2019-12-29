// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_ARRAYOPERATIONS_HPP
#define AUXILIARY_ARRAYOPERATIONS_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>

namespace HPM::auxiliary
{
    namespace
    {
        //!
        //! \brief Applies a callable to each element of an array.
        //!
        //! \tparam T the type of the array elements
        //! \tparam FuncT the type of the callable
        //! \tparam I an index parameter pack
        //! \param in the input array
        //! \param func a callable taking one argument of type `T`
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return the transformed array: elements are the return values of the callable applied to the input
        //!
        template <typename T, typename FuncT, std::size_t... I>
        inline constexpr auto TransformArray(const std::array<T, sizeof...(I)>& in, const FuncT& func, std::index_sequence<I...>)
        {
            static_assert(std::is_invocable_v<FuncT, T>, "error: callable is not invocable");

            return std::array<decltype(func(T{})), sizeof...(I)>{func(in[I])...};
        }
    } // namespace

    //!
    //! \brief Applies a callable to each element of an array.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \tparam FuncT the type of the callable
    //! \param in the input array
    //! \param func a callable taking one argument of type `T`
    //! \return the transformed array: elements are the return values of the callable applied to the input
    //!
    template <typename T, std::size_t N, typename FuncT>
    inline constexpr auto TransformArray(const std::array<T, N>& in, const FuncT& func)
    {
        return TransformArray(in, func, std::make_index_sequence<N>{});
    }

    namespace
    {
        //!
        //! \brief Combines two arrays by applying a callable element wise.
        //!
        //! \tparam T the type of the array elements
        //! \tparam FuncT the type of the callable
        //! \tparam I an index parameter pack
        //! \param in_1 the input array
        //! \param in_2 another input array
        //! \param func a callable taking one argument of type `T`
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return the combined array: elements are the return values of the callable applied to the inputs
        //!
        template <typename T, typename FuncT, std::size_t... I>
        inline constexpr auto TransformArray(const std::array<T, sizeof...(I)>& in_1, const std::array<T, sizeof...(I)>& in_2, const FuncT& func, std::index_sequence<I...>)
        {
            static_assert(std::is_invocable_v<FuncT, T, T>, "error: callable is not invocable");

            return std::array<decltype(func(T{}, T{})), sizeof...(I)>{func(in_1[I], in_2[I])...};
        }
    } // namespace

    //!
    //! \brief Combines two arrays by applying a callable element wise.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \param in_1 the input array
    //! \param in_2 another input array
    //! \param func a callable taking one argument of type `T`
    //! \return the combined array: elements are the return values of the callable applied to the inputs
    //!
    template <typename T, std::size_t N, typename FuncT>
    inline constexpr auto TransformArray(const std::array<T, N>& in_1, const std::array<T, N>& in_2, const FuncT& func)
    {
        return TransformArray(in_1, in_2, func, std::make_index_sequence<N>{});
    }

    namespace
    {
        //!
        //! \brief Extract a sub-array from a given array.
        //!
        //! This function extracts a selection of elements from a given array.
        //!
        //! \tparam T the type of the array elements
        //! \tparam N the number of array elements
        //! \tparam I an index parameter pack
        //! \param in the input array
        //! \param selection an array of array indices specifying the sub-array
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return an array with elements taken from the input according to the selection
        //!
        template <typename T, std::size_t N, std::size_t... I>
        inline constexpr auto GetSubArray(const std::array<T, N>& in, const std::array<std::size_t, sizeof...(I)>& selection, std::index_sequence<I...>)
        {
            static_assert(sizeof...(I) <= N, "error: sub-array is larger than the input array");

            return std::array<T, sizeof...(I)>{in[selection[I]]...};
        }
    } // namespace

    //!
    //! \brief Extract a sub-array from a given array.
    //!
    //! This function extracts a selection of elements from a given array.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \tparam M the length of the sub-array
    //! \param in the input array
    //! \param selection an array of array indices specifying the sub-array
    //! \return an array with elements taken from the input according to the selection
    //!
    template <typename T, std::size_t N, std::size_t M>
    inline constexpr auto GetSubArray(const std::array<T, N>& in, const std::array<std::size_t, M>& selection)
    {
        return GetSubArray(in, selection, std::make_index_sequence<M>{});
    }

    //!
    //! \brief Reverse the order of the array elements.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \param in the input array
    //! \return an array with all elements in reverse order
    //!
    template <typename T, std::size_t N>
    inline constexpr auto GetReverseOrder(const std::array<T, N>& in)
    {
        std::array<T, N> out{in};

        ConstexprFor<N>([&in, &out](const auto I) { out[I] = in[N - 1 - I]; });

        return out;
    }

    //!
    //! \brief Fill an array from an input stream.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \param input_stream a data stream
    //! \param a the array to be filled
    //! \return the input stream
    //!
    template <typename T, std::size_t N>
    auto operator>>(std::istream& input_stream, std::array<T, N>& a) -> std::istream&
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            input_stream >> a[i];
        }

        return input_stream;
    }

    //!
    //! \brief Write all elements of an array into an output stream.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //! \param output_stream a data stream
    //! \param a the array to be written
    //! \return the output stream
    //!
    template <typename T, std::size_t N>
    auto operator<<(std::ostream& output_stream, const std::array<T, N>& a) -> std::ostream&
    {
        std::cout << "( ";
        for (std::size_t i = 0; i < N; ++i)
        {
            output_stream << a[i] << " ";
        }
        std::cout << ")";

        return output_stream;
    }
} // namespace HPM::auxiliary

#endif
