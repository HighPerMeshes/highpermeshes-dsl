// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_LOOPTYPES_FREELOOPS_FOREACHENTITY_HPP
#define DSL_LOOPTYPES_FREELOOPS_FOREACHENTITY_HPP

#include <utility>

#include <HighPerMeshes/dsl/meshes/Range.hpp>

namespace HPM
{
    //!
    //! ForEachEntity iterates over all entities of a given dimension within a given mesh::Range
    //! and calls the invocable `f` for each iteration.
    //!
    //! For example, given a range of entities `r`, `ForEachEntity(r, [](const auto& e) {})` iterates over all entities in `r`
    //!
    //! \tparam
    //! EntityDimension The dimensionality of the entities within the mesh this function considers.
    //!
    //! \attention
    //! `f` must be invocable with the requested entities
    //!
    //! \see
    //! HPM::mesh::Range
    //!
    template <std::size_t EntityDimension, typename MeshT, typename LoopBody>
    auto ForEachEntity(const ::HPM::mesh::Range<EntityDimension, MeshT>& range, LoopBody&& loop_body)
    {
        for (const auto& entity : range.GetEntities())
        {
            std::forward<LoopBody>(loop_body)(entity);
        }
    }
} // namespace HPM

#endif