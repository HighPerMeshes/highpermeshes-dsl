// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_CONSTEXPRARRAY_HPP
#define COMMON_CONSTEXPRARRAY_HPP

#include <array>
#include <cstdint>

#include <HighPerMeshes/auxiliary/ConstexprIfElse.hpp>

namespace HPM::dataType
{
    //!
    //! \brief A fixed size compile-time constant array type.
    //!
    //! Array elements are compile-time constants.
    //! Different values and array sizes result in different types.
    //! Element access happens through an `std::array` member variable.
    //!
    //! Usage:
    //! ```
    //!     auto a_1 = ConstexprArray<int, 1, 3, 3, 1>{};                       -> {1, 3, 3, 1}
    //!     auto a_2 = ConstexprArray<int, 1, 3, 3, 1>::Set<2, 6>();            -> {1, 3, 6, 1}
    //!     auto a_3 = ConstexprArray<int, 1, 3, 3, 1>::Clear<2>();             -> {1, 3, 0, 1}
    //!     auto a_4 = ConstexprArray<int, 1, 3, 3, 1>::ClearAll<2>();          -> {1, 3, 0, 0}
    //!     auto a_5 = ConstexprArray<int, 1, 3, 0, 1>::ExtractMask();          -> {1, 1, 0, 1}
    //!     auto a_6 = ConstexprArray<int, 1, 3, 0, 1>::Add(a_1);               -> {2, 6, 3, 2}
    //!     auto a_7 = ConstexprArray<int, 1, 3, 0, 1>::Add(a_1).ExtractMask(); -> {1, 1, 1, 1}
    //! ```
    //!
    //! \tparam T the type of the array elements
    //! \tparam Value a parameter pack defining the array elements
    //!
    template <typename T, T... Value>
    class ConstexprArray
    {
        // move the parameter pack into an array for element access.
        static constexpr std::array<T, sizeof...(Value)> value{Value...};

        //!
        //! \brief Assign a new value to an array element.
        //!
        //! \tparam Index the index position of the element
        //! \tparam NewValue the new value
        //! \tparam I an index sequence used for the element access
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return a new `ConstexprArray` variable (of a different data type)
        //!
        template <std::size_t Index, T NewValue, std::size_t... I>
        static constexpr auto Set(std::index_sequence<I...>)
        {
            return ConstexprArray<T, ::HPM::auxiliary::ConstexprIfElse<(I == Index)>(NewValue, value[I])...>{};
        }

        //!
        //! \brief Clear all array elements starting from a given position.
        //!
        //! Array elements from zero to that position (exclusive) remain the same.
        //! All other elements are set to zero.
        //!
        //! \tparam StartIndex the index position from which zeroing will start
        //! \tparam I an index sequence used for the element access
        //! \param unnamed (not used) an index sequence for template parameter deduction
        //! \return a new `ConstexprArray` variable (of a different data type)
        //!
        template <std::size_t StartIndex, std::size_t... I>
        static constexpr auto ClearAll(std::index_sequence<I...>)
        {
            return ConstexprArray<T, ::HPM::auxiliary::ConstexprIfElse<(I >= StartIndex)>(0, value[I])...>{};
        }

        public:
        using ArrayT = typename std::remove_const<decltype(value)>::type;
        static constexpr bool IsConstexprArray = true;

        //!
        //! \brief Return the size of the array.
        //!
        //! \return the size of the array
        //!
        static constexpr auto Size() { return sizeof...(Value); }

        //!
        //! \brief Return a reference to the content of the array.
        //!
        //! \return a reference to the content of the array
        //!
        static constexpr auto Get() -> const ArrayT& { return value; }

        //!
        //! \brief Assign a new value to an array element.
        //!
        //! \tparam Index the index position of the element
        //! \tparam NewValue the new value
        //! \return a new `ConstexprArray` variable (of a different data type)
        //!
        template <std::size_t Index, T NewValue>
        static constexpr auto Set()
        {
            return Set<Index, NewValue>(std::make_index_sequence<sizeof...(Value)>{});
        }

        //!
        //! \brief Assign zero to (or clear) the array element at position `Index`.
        //!
        //! \tparam Index the index position of the element
        //! \return a new `ConstexprArray` variable (of a different data type)
        //!
        template <std::size_t Index>
        static constexpr auto Clear()
        {
            return Set<Index, 0>();
        }

        //!
        //! \brief Clear all array elements starting from a given position.
        //!
        //! Array elements from zero to that position (exclusive) remain the same.
        //! All other elements are set to zero.
        //!
        //! \tparam StartIndex the index position from which zeroing will start
        //! \return a new `ConstexprArray` variable (of a different data type)
        //!
        template <std::size_t StartIndex = 0>
        static constexpr auto ClearAll()
        {
            return ClearAll<StartIndex>(std::make_index_sequence<sizeof...(Value)>{});
        }

        //!
        //! \brief Transform this array into a mask.
        //!
        //! The mask holds `1` for all elements that have a value different form `0`, and `0` otherwise.
        //!
        //! \return a mask
        static constexpr auto ExtractMask() { return ConstexprArray<std::size_t, ::HPM::auxiliary::ConstexprIfElse<Value != 0>(1, 0)...>{}; }

        //!
        //! \brief Accumulate all array elements using the `+` operation.
        //!
        //! \return the sum of all array elements
        //!
        static constexpr auto Sum() { return (Value + ...); }

        //!
        //! \brief Arithmetic operations.
        //!
        //! \return the result of the arithmetic operation, specifying a new `ConstexprArray` type
        //!
#define MACRO(OP_NAME, OP)                                                                                                                                                                                                 \
template <T... C_ValueOther>                                                                                                                                                                                           \
static constexpr auto OP_NAME(const ConstexprArray<T, C_ValueOther...>&)                                                                                                                                               \
{                                                                                                                                                                                                                      \
    return ConstexprArray<T, Value OP C_ValueOther...>{};                                                                                                                                                              \
}

        MACRO(Add, +)
        MACRO(Sub, -)
        MACRO(Mul, *)
        MACRO(Div, /)

#undef MACRO

        //!
        //! \brief Access a specific array element.
        //!
        //! \tparam Index the index position of the element
        //! \return a constant reference to the element
        //!
        template <std::size_t Index>
        static constexpr auto At() -> const T&
        {
            static_assert(Index < Size(), "error: out of bounds data access");

            return value[Index];
        }

        //!
        //! \brief Access a specific array element.
        //!
        //! \param index the index position of the element
        //! \return a constant reference to the element
        //!
        static constexpr auto At(const std::size_t index) -> const T& { return value[index]; }
    };

    //!
    //! \brief A constant array type.
    //!
    //! \tparam T the type of the array elements
    //! \tparam N the number of array elements
    //!
    template <typename T, std::size_t N>
    class ConstArray
    {
        const std::array<T, N> value;

        public:
        static constexpr bool IsConstexprArray = false;

        //!
        //! \brief Constructor
        //!
        //! \tparam Value parameter pack of value types
        //! \param value values to be assigned to the array
        //!
        template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == N), int> = 0>
        constexpr ConstArray(Args&&... args) : value{static_cast<T>(args)...}
        {
        }

        //!
        //! \brief Constructor.
        //!
        //! \param value an array of size `N`
        //!
        constexpr ConstArray(const std::array<T, N>& value) : value(value) {}

        //!
        //! \brief Return the size of the array.
        //!
        //! \return the size of the array
        //!
        static constexpr auto Size() { return N; }

        //!
        //! \brief Return a reference to the content of the array.
        //!
        //! \return a reference to the content of the array
        //!
        constexpr auto Get() const -> const std::array<T, N>& { return value; }

        //!
        //! \brief Access a specific array element.
        //!
        //! \tparam Index the index position of the element
        //! \return a constant reference to the element
        //!
        template <std::size_t Index>
        constexpr auto At() const -> const T&
        {
            static_assert(Index < Size(), "error: out of bounds data access");

            return value[Index];
        }

        //!
        //! \brief Access a specific array element.
        //!
        //! \param index the index position of the element
        //! \return a constant reference to the element
        //!
        constexpr auto At(const std::size_t index) const -> const T& { return value[index]; }
    };
} // namespace HPM::dataType

#endif