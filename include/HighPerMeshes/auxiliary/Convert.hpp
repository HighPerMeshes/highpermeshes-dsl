// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_CONVERT_HPP
#define AUXILIARY_CONVERT_HPP

#include <stdexcept>
#include <utility>

namespace HPM::auxiliary
{
    //!
    //! \return Safely cast argument of type `From` to `To`
    //! \throws If `From` can not be converted to `To`
    //!
    template <typename To, typename From>
    auto Convert(From&& in)
    {
        const To out = static_cast<To>(std::forward<From>(in));

        if (static_cast<std::decay_t<From>>(out) != in)
        {
            throw std::invalid_argument{"Cannot convert expression"};
        }

        return out;
    }
} // namespace HPM::auxiliary

#endif