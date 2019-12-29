// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_CONSTEXPRIFELSE_HPP
#define AUXILIARY_CONSTEXPRIFELSE_HPP

#include <type_traits>

namespace HPM::auxiliary
{
    //!
    //! \brief Returns the result of an `if`-`else` statement.
    //!
    //! Given a compile time predicate, this function returns either its 1st or its 2nd argument associated with the `if`- and `else`-branch, respectively.
    //! If any of the arguments is a callable, its return value is returned.
    //! The callable must take no arguments and can return `void`.
    //!
    //! (Value) Equivalent to:
    //! ```
    //!     if (Predicate) return a; 
    //!     else return b;
    //! ```
    //! (Callable) Equivalent to:
    //! ```
    //!     if (Predicate) return a(); 
    //!     else return b();
    //! ```
    //!
    //! \tparam Predicate a compile time predicate that is either `true` or `false`
    //! \tparam T_1 type of the `if`-branch argument argument (can be a callable)
    //! \tparam T_2 type of the `else`-branch argument argument (can be a callable)
    //! \param a `if`-branch argument
    //! \param b `else`-branch argument
    //! \return either `a` or `b` or their return values
    //!
    template <bool Predicate, typename T_1, typename T_2>
    constexpr auto ConstexprIfElse(const T_1& a, const T_2& b)
    {
        if constexpr (Predicate)
        {
            if constexpr (std::is_invocable_v<T_1>)
            {
                return a();
            }
            else
            {
                return a;
            }
        }
        else
        {
            if constexpr (std::is_invocable_v<T_2>)
            {
                return b();
            }
            else
            {
                return b;
            }
        }
    }
} // namespace HPM::auxiliary

#endif