// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_LOOPTYPES_FREELOOPS_FOREACH_HPP
#define DSL_LOOPTYPES_FREELOOPS_FOREACH_HPP

#include <utility>

#include <HighPerMeshes/common/Iterator.hpp>

namespace HPM
{
    //!
    //! ForEach executes the invocable `f` for each integer in `range`.
    //!
    //! \attention
    //! `f` must be invocable with an instance of `IntegerT`.
    //!
    //! \see
    //! HPM::iterator::Range
    //!
    //! \example
    //! ForEach( Range { 0, 5 },
    //!          [](auto i) { });
    //!
    template <typename IntegerT, IntegerT StepSize, typename LoopBody>
    auto ForEach(iterator::Range<IntegerT, StepSize>&& range, LoopBody&& loop_body)
    {
        for (auto iter : std::move(range))
        {
            std::forward<LoopBody>(loop_body)(iter);
        }
    }

    //!
    //! ForEach executes the invocable `f` for each integer in the range [0, times).
    //!
    //! \attention
    //! `f` must be invocable with an instance of `IntegerT`.
    //!
    //! \example
    //! ForEach( 5,
    //!          [](auto i) { });
    //!
    template <typename IntegerT, typename LoopBody>
    auto ForEach(IntegerT times, LoopBody&& loop_body)
    {
        ForEach(iterator::Range{times}, std::forward<LoopBody>(loop_body));
    }
} // namespace HPM

#endif