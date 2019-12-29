// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DISPATCHERS_SEQUENTIALDISPATCHER_HPP
#define DSL_DISPATCHERS_SEQUENTIALDISPATCHER_HPP

#include <utility>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/dispatchers/Dispatcher.hpp>

namespace HPM
{
    //!
    //! \brief Provides a class to execute MeshLoops sequentially.
    //!
    //! SequentialDispatcher provides a sequential implementation for the Dispatcher base class that allows
    //! executing MeshLoops.
    //!
    //! Usage:
    //! Input: mesh, buffer
    //! \code{.cpp}
    //! SequentialDispatcher { }.Execute(
    //!     ForEachEntity(
    //!     mesh.GetEntityRange<Mesh::CellDimension>(),
    //!     std::tuple(Write(Cell(buffer))),
    //!     [&] (const auto& cell, auto &&, auto lvs)
    //!     {
    //!         //...
    //!     })
    //! );
    //! \endcode
    //! \see
    //! Dispatcher
    //! \note
    //! CRTP
    //!
    class SequentialDispatcher : public Dispatcher<SequentialDispatcher>
    {
      public:
        //! Implementation of the dispatch function
        //! \see HPM::Dispatcher
        template <typename... MeshLoops, typename IntegerT>
        auto Dispatch(iterator::Range<IntegerT> range, MeshLoops&&... mesh_loops)
        {
            for (auto step : range)
            {
                ([step](auto&& mesh_loop) {
                    const auto& range = mesh_loop.entity_range;
                    mesh_loop.loop(range.GetEntities(), mesh_loop.access_definitions, [&mesh_loop, &step](auto&& entity, auto& localVectors) { mesh_loop.loop_body(entity, step, localVectors); });
                } (std::forward<MeshLoops>(mesh_loops)),...);
            }
        }
    };
} // namespace HPM

#endif