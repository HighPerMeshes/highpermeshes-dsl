// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILILARY_MATH_HPP
#define AUXILILARY_MATH_HPP

#include <array>
#include <cmath>
#include <cstdint>

namespace HPM::math
{
    //!
    //! \brief Calculates the factorial function.
    //!
    //! \param n an unsigned integer
    //! \return `n!`
    //!
    constexpr auto Factorial(const std::size_t n)
    {
        std::size_t result = 1;

        for (std::size_t i = 1; i <= n; ++i)
        {
            result *= i;
        }

        return result;
    }

    //!
    //! \brief Get a specific element among all combinations of `M` numbers out of a set of `N>M` numbers.
    //!
    //! The number of combinations is `N!/(M!*(N-M)!)`.
    //! All combinations are deduced and one of them is selected in the end.
    //!
    //! \tparam M the number of elements being selected
    //! \tparam N the overall number of elements
    //! \param unnamed (hereafter `selection`) the index of any of the combinations
    //! \return the combination according to `selection`
    //!
    template <std::size_t M, std::size_t N>
    constexpr auto GetCombination(const std::size_t)
    {
        return std::array<std::size_t, M>{};
    }

    template <>
    constexpr auto GetCombination<1, 1>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(1) / (Factorial(1) * (Factorial(0)))> Combinations = {{{{0}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<1, 2>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(2) / (Factorial(1) * (Factorial(1)))> Combinations = {{{{0}}, {{1}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<2, 2>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 2>, Factorial(2) / (Factorial(2) * (Factorial(0)))> Combinations = {{{{0, 1}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<1, 3>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(3) / (Factorial(1) * (Factorial(2)))> Combinations = {{{{0}}, {{1}}, {{2}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<2, 3>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 2>, Factorial(3) / (Factorial(2) * (Factorial(1)))> Combinations = {{{{0, 1}}, {{0, 2}}, {{1, 2}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<3, 3>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 3>, Factorial(3) / (Factorial(3) * (Factorial(0)))> Combinations = {{{{0, 1, 2}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<1, 4>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(4) / (Factorial(1) * (Factorial(3)))> Combinations = {{{{0}}, {{1}}, {{2}}, {{3}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<2, 4>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 2>, Factorial(4) / (Factorial(2) * (Factorial(2)))> Combinations = {{{{0, 1}}, {{0, 2}}, {{0, 3}}, {{1, 2}}, {{1, 3}}, {{2, 3}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<3, 4>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 3>, Factorial(4) / (Factorial(3) * (Factorial(1)))> Combinations = {{{{0, 1, 2}}, {{0, 1, 3}}, {{1, 2, 3}}, {{0, 2, 3}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<4, 4>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 4>, Factorial(4) / (Factorial(4) * (Factorial(0)))> Combinations = {{{{0, 1, 2, 3}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<1, 5>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(5) / (Factorial(1) * (Factorial(4)))> Combinations = {{{{0}}, {{1}}, {{2}}, {{3}}, {{4}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<2, 5>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 2>, Factorial(5) / (Factorial(2) * (Factorial(3)))> Combinations = {
            {{{0, 1}}, {{0, 2}}, {{0, 3}}, {{0, 4}}, {{1, 2}}, {{1, 3}}, {{1, 4}}, {{2, 3}}, {{2, 4}}, {{3, 4}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<3, 5>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 3>, Factorial(5) / (Factorial(3) * (Factorial(2)))> Combinations = {
            {{{0, 1, 2}}, {{0, 1, 3}}, {{0, 1, 4}}, {{0, 2, 3}}, {{0, 2, 4}}, {{0, 3, 4}}, {{1, 2, 3}}, {{1, 2, 4}}, {{1, 3, 4}}, {{2, 3, 4}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<4, 5>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 4>, Factorial(5) / (Factorial(4) * (Factorial(1)))> Combinations = {{{{0, 1, 2, 3}}, {{0, 1, 2, 4}}, {{0, 1, 3, 4}}, {{0, 2, 3, 4}}, {{1, 2, 3, 4}}}};

        return Combinations[selection];
    }

    template <>
    constexpr auto GetCombination<5, 5>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 5>, Factorial(5) / (Factorial(5) * (Factorial(0)))> Combinations = {{{{0, 1, 2, 3, 4}}}};

        return Combinations[selection];
    }

    //!
    //! \brief Get a specific element among all permutations of the first `N-1` unsigned integers.
    //!
    //! The number of permutations is `N!`.
    //! All permutations are deduced and one of them is selected in the end.
    //!
    //! \tparam N the number of elements to be permuted
    //! \param unnamed (hereafter `selection`) the index of any of the combinations
    //! \return the permutation according to `selection`
    //!
    template <std::size_t N>
    constexpr auto GetPermutation(const std::size_t)
    {
        return std::array<std::size_t, N>{};
    }

    //!
    //! \brief Get a the sign of a specific element among all permutations of the first `N-1` unsigned integers.
    //!
    //! The number of permutations is `N!`.
    //! All signs of permutations are deduced and one of them is selected in the end.
    //!
    //! \tparam N the number of elements to be permuted
    //! \param unnamed (hereafter `selection`) the index of any of the permutations
    //! \return the sign of the permutation according to `selection`
    //!
    template <std::size_t N>
    constexpr auto GetSignOfPermutation(const std::size_t)
    {
        return std::int8_t{1};
    }

    template <>
    constexpr auto GetPermutation<1>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 1>, Factorial(1)> Permutations = {{{{0}}}};

        return Permutations[selection];
    }

    template <>
    constexpr auto GetSignOfPermutation<1>(const std::size_t)
    {
        return std::int8_t{1};
    }

    template <>
    constexpr auto GetPermutation<2>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 2>, Factorial(2)> Permutations = {{{{0, 1}}, {{1, 0}}}};

        return Permutations[selection];
    }

    template <>
    constexpr auto GetSignOfPermutation<2>(const std::size_t selection)
    {
        constexpr std::array<std::int8_t, Factorial(2)> C_Sign = {{1, -1}};

        return C_Sign[selection];
    }

    template <>
    constexpr auto GetPermutation<3>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 3>, Factorial(3)> Permutations = {{{{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}}, {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

        return Permutations[selection];
    }

    template <>
    constexpr auto GetSignOfPermutation<3>(const std::size_t selection)
    {
        constexpr std::array<std::int8_t, Factorial(3)> C_Sign = {{1, -1, -1, 1, 1, -1}};

        return C_Sign[selection];
    }

    template <>
    constexpr auto GetPermutation<4>(const std::size_t selection)
    {
        constexpr std::array<std::array<std::size_t, 4>, Factorial(4)> Permutations = {
            {{{0, 1, 2, 3}}, {{0, 1, 3, 2}}, {{0, 2, 1, 3}}, {{0, 2, 3, 1}}, {{0, 3, 1, 2}}, {{0, 3, 2, 1}}, {{1, 0, 2, 3}}, {{1, 0, 3, 2}}, {{1, 2, 0, 3}}, {{1, 2, 3, 0}}, {{1, 3, 0, 2}}, {{1, 3, 2, 0}},
                {{2, 0, 1, 3}}, {{2, 0, 3, 1}}, {{2, 1, 0, 3}}, {{2, 1, 3, 0}}, {{2, 3, 0, 1}}, {{2, 3, 1, 0}}, {{3, 0, 1, 2}}, {{3, 0, 2, 1}}, {{3, 1, 0, 2}}, {{3, 1, 2, 0}}, {{3, 2, 0, 1}}, {{3, 2, 1, 0}}}};

        return Permutations[selection];
    }

    template <>
    constexpr auto GetSignOfPermutation<4>(const std::size_t selection)
    {
        constexpr std::array<std::int8_t, Factorial(4)> C_Sign = {{1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1}};

        return C_Sign[selection];
    }
} // namespace HPM::math

#endif