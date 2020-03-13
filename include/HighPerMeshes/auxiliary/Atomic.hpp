// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_ATOMIC_HPP
#define AUXILIARY_ATOMIC_HPP

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace HPM::atomic
{
    //!
    //! \brief A compare and swap implementation for words up to 8 byte size.
    //!
    //! The implementation uses `__sync_bool_compare_and_swap` for integer valued elements up to 8 bytes,
    //! and floating point numbers of type `float` and `double`.
    //! For the latter, a reinterpret cast is applied.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable to be updated
    //! \param expected_value the value that is expected/assumed before the update
    //! \param new_value the value to be assigned to the variable (iff it holds the expected value)
    //! \return a `bool` that signals success (`true`) or no success (`false`)
    //!
    template <typename T>
    auto CompareAndSwap(T& variable, const T& expected_value, const T& new_value) -> bool
    {
        static_assert(sizeof(T) <= 8, "error: atomic operations on words larger than 8 byte are not allowed");

        if constexpr (std::is_integral_v<T>)
        {
            return __sync_bool_compare_and_swap(&variable, expected_value, new_value);
        }
        else
        {
            using IntegerType = std::conditional_t<sizeof(T) == 1, std::int8_t, std::conditional_t<sizeof(T) == 2, std::int16_t, std::conditional_t<sizeof(T) == 4, std::int32_t, std::int64_t>>>;

            return CompareAndSwap(reinterpret_cast<IntegerType&>(variable), reinterpret_cast<const IntegerType&>(expected_value), reinterpret_cast<const IntegerType&>(new_value));
        }
    }

    //!
    //! \brief An atomic exchange operation.
    //!
    //! This function exchanges the value stored in a variable by a new one.
    //! The new value is assigned in any case.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable to be updated
    //! \param new_value the value to be assigned to the `variable`
    //! \return the old value before the assignment
    //!
    template <typename T>
    auto Exchange(T& variable, const T& new_value)
    {
        T old_value;

        do
        {
            old_value = variable;
        } while (!CompareAndSwap(variable, old_value, new_value));

        return old_value;
    }

    //!
    //! \brief Execute a function atomically.
    //!
    //! This function executes a callable on a variable atomically.
    //! The argument and return type of the callable and the variable type must be the same.
    //!
    //! \tparam T the element type
    //! \tparam FuncT the type of the callable
    //! \param variable a reference to the variable the callable executes on
    //! \param func a callable with one argument of type `T`
    //! \return the old value before the execution
    //!
    template <typename T, typename FuncT>
    auto FetchAndExecute(T& variable, const FuncT& func)
    {
        static_assert(std::is_invocable_v<FuncT, T>, "error: callable is not invocable");
        static_assert(std::is_same_v<T, decltype(func(T{}))>, "error: callable return type and variable type do not match");

        T old_value;

        do
        {
            old_value = variable;
        } while (!CompareAndSwap(variable, old_value, func(old_value)));

        return old_value;
    }

    //!
    //! \brief Execute a function atomically.
    //!
    //! This function executes a function on a variable atomically.
    //! The argument and return type of the callable and the variable type must be the same.
    //!
    //! \tparam T the element type
    //! \tparam FuncT the type of the callable
    //! \param variable a reference to the variable the callable executes on
    //! \param func a callable with one argument of type `T`
    //! \return the new value after the execution
    //!
    template <typename T, typename FuncT>
    auto ExecuteAndFetch(T& variable, const FuncT& func)
    {
        static_assert(std::is_invocable_v<FuncT, T>, "error: callable is not invocable");
        static_assert(std::is_same_v<T, decltype(func(T{}))>, "error: callable return type and variable type do not match");

        T old_value;
        T new_value;

        do
        {
            old_value = variable;
            new_value = func(old_value);
        } while (!CompareAndSwap(variable, old_value, new_value));

        return new_value;
    }

    //!
    //! \brief An atomic add operation.
    //!
    //! This function adds a value to a variable.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable to add to
    //! \param value the value to be added to `variable`
    //! \return the old value before the add
    //!
    template <typename T>
    auto FetchAndAdd(T& variable, const T& value)
    {
        return FetchAndExecute(variable, [&value](const T& current_value) { return current_value + value; });
    }

    //!
    //! \brief An atomic add operation.
    //!
    //! This function adds a value to a variable.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable to add to
    //! \param value the value to be added to `variable`
    //! \return the old value after the add
    //!
    template <typename T>
    auto AddAndFetch(T& variable, const T& value)
    {
        return ExecuteAndFetch(variable, [&value](const T& current_value) { return current_value + value; });
    }

    template <typename T>
    auto AtomicAdd(T& variable, const T& value)
    {
        return AddAndFetch(variable, value);
    }

    //!
    //! \brief An atomic min operation.
    //!
    //! This function sets `variable` to the minimum of its current value and the proposed minimum.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable holding the minimum
    //! \param proposed_min the value proposed as minimum
    //! \return the old value before the operation
    //!
    template <typename T>
    auto Min(T& variable, const T& proposed_min)
    {
        return FetchAndExecute(variable, [&proposed_min](const T& current_value) { return std::min(current_value, proposed_min); });
    }

    template <typename T>
    auto AtomicMin(T& variable, const T& proposed_min)
    {
        return Min(variable, proposed_min);
    }

    //!
    //! \brief An atomic max operation.
    //!
    //! This function sets `variable` to the maximum of its current value and the proposed maximum.
    //!
    //! \tparam T the element type
    //! \param variable a reference to the variable holding the maximum
    //! \param proposed_min the value proposed as maximum
    //! \return the old value before the operation
    //!
    template <typename T>
    auto Max(T& variable, const T& proposed_max)
    {
        return FetchAndExecute(variable, [&proposed_max](const T& current_value) { return std::max(current_value, proposed_max); });
    }

    template <typename T>
    auto AtomicMax(T& variable, const T& proposed_max)
    {
        return Max(variable, proposed_max);
    }
} // namespace HPM::atomic

    // Forward declaration.
namespace HPM::dataType
{
    template <typename, std::size_t>
    class Vec;
} // namespace dataType

namespace HPM::atomic
{
    using ::HPM::dataType::Vec;

    //!
    //! \brief An atomic add operation for vectors of fixed size.
    //!
    //! This function adds a value to each vector component.
    //!
    //! \tparam T the element type of the vector
    //! \tparam N the number of elements in the vector
    //! \param variable a reference to the variable to add to
    //! \param value the value to be added to `variable`
    //! \return the old value before the add
    //!
    template <typename T, std::size_t N>
    auto FetchAndAdd(Vec<T, N>& variable, const Vec<T, N>& value)
    {
        Vec<T, N> return_value;

        for (std::size_t i = 0; i < N; ++i)
        {
            return_value[i] = FetchAndAdd(variable[i], value[i]);
        }

        return return_value;
    }
} // namespace HPM::atomic

#endif