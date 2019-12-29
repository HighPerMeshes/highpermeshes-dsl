// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_LOOPTYPES_FREELOOPS_FOREACHSUBENTITY_HPP
#define DSL_LOOPTYPES_FREELOOPS_FOREACHSUBENTITY_HPP

#include <utility>

namespace HPM
{
    //!
    //! ForEachSubEntity iterates over all direct children of a given entity
    //! and calls the invocable `f` with the given child.
    //!
    //! For example, given an edge e, ForEachSubEntity(e, [](auto) {}) invokes `f` for both nodes in `e`.
    //!
    //! \attention
    //! `f` must be invocable with the requested sub-entities
    //!
    //! \see
    //! HPM::entity::Topology
    //!
    template <typename EntityT, typename LoopBody>
    auto ForEachSubEntity(const EntityT& entity, LoopBody&& loop_body)
    {
        for (const auto& subEntity : entity.GetTopology().GetSubEntities())
        {
            std::forward<LoopBody>(loop_body)(subEntity);
        }
    }
} // namespace HPM

#endif