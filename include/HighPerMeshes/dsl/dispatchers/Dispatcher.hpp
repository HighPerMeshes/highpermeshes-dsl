// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DISPATCHERS_DISPATCHER_HPP
#define DSL_DISPATCHERS_DISPATCHER_HPP

#include <utility>

#include <HighPerMeshes/common/Iterator.hpp>

namespace HPM
{
    //! Dispatcher allows dispatching any number of `mesh_loops` any number of times to a given dispatcher implementation.
    //!
    //! \tparam
    //! DispatchTo defines a sub-class that Executes all specified mesh_loops somehow by implementing the `dispatch` method
    //!
    //! \note
    //! CRTP
    template <typename DispatchTo>
    class Dispatcher
    {
      public:
        //! \brief
        //! Executes any number of `mesh_loops` once.
        //! \see
        //! HPM::MeshLoop
        template <typename... MeshLoops>
        auto Execute(MeshLoops&&... mesh_loops)
        {
            Execute(iterator::Range{1}, std::forward<MeshLoops>(mesh_loops)...);
        }

        //! \brief Executes any number of `mesh_loops` for each value in the range `range`.
        //!
        //! The dispatcher promises to Execute the mesh_loops by iterating over all its entities saved in `consideredEntities` in a wat specified
        //! by the `loop`. While iterating it Executes the `operation` that requires the specified `accesses`.
        //!
        //! Usage:
        //! Input: mesh, buffer
        //! \code{.cpp}
        //! Dispatcher<SubClass> {}.Execute(
        //!     HPM::ForEachEntity(
        //!     mesh.GetEntityRange<Mesh::CellDimension>(),
        //!     std::tuple(Write(Cell(buffer))),
        //!     [&] (const auto& cell, auto &&, auto lvs)
        //!     {
        //!         //...
        //!     })
        //! );
        //! \endcode
        //!
        //! \param range specified the range of time steps that the mesh_loops are repeated
        //! \param mesh_loops A number of mesh_loops as defined by HPM::MeshLoop.
        //! \see
        //! HPM::MeshLoop
        //! \see
        //! HPM::iterator::Range
        template <typename... MeshLoops, typename IntegerT>
        auto Execute(iterator::Range<IntegerT> range, MeshLoops&&... mesh_loops)
        {
            static_cast<DispatchTo*>(this)->Dispatch(range, std::forward<MeshLoops>(mesh_loops)...);
        }
    };
} // namespace HPM

#endif