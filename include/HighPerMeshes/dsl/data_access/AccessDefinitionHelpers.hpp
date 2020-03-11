// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_ACCESSDEFINITIONHELPERS_HPP
#define DSL_DATAACCESS_ACCESSDEFINITIONHELPERS_HPP

#include <HighPerMeshes/dsl/data_access/AccessDefinition.hpp>
#include <HighPerMeshes/dsl/data_access/AccessMode.hpp>
#include <HighPerMeshes/dsl/data_access/AccessPatterns.hpp>
#include <HighPerMeshes/dsl/data_access/DofRequests.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsAccessDefinitions.hpp>

namespace HPM
{
    //! \name
    //! AccessPatternHelpers
    //!
    //! Given a buffer, these functions create an AccessDefinition with ReadWrite access and their corresponding AccessPattern
    //! \see
    //! AccessPattern
    //! \todo {I think this indirect inference of the Mesh from the buffer is kind of hidden, but I do not now a better solution atm - Stefan G. 23.8.19}
    //!
    //! \{
    template <typename BufferT>
    auto Global(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, MeshT::CellDimension + 1>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto Cell(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, MeshT::CellDimension>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto NeighboringMeshElementOrSelf(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::NeighboringMeshElementOrSelfPattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, MeshT::CellDimension>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto ContainingMeshElement(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::ContainingMeshElementPattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, MeshT::CellDimension>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto All(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestAll<MeshT::CellDimension + 2>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto Node(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, 0>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto Edge(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, 1>, ReadWriteConstant};
    }
    template <typename BufferT>
    auto Face(BufferT& buffer)
    {
        using MeshT = typename BufferT::MeshT;
        return AccessDefinition{&buffer, AccessPatterns::SimplePattern, dof_requests::RequestEquals<MeshT::CellDimension + 2, MeshT::CellDimension - 1>, ReadWriteConstant};
    }
    //! \}

    //! \name
    //! DataAccessHelpers
    //!
    //! Given an AccessDefinition, these functions create an AccessDefinition with the given AccessPattern and their corresponding DataAccess
    //!
    //! \see
    //! DataAccess
    //!
    //! \{
    auto Read = [](auto&& access_definition) {
        static_assert(IsAccessDefinition<std::decay_t<decltype(access_definition)>>);
        return AccessDefinition{std::move(access_definition.buffer), std::move(access_definition.pattern), access_definition.Dofs, ReadConstant};
    };
    auto Write = [](auto&& access_definition) {
        static_assert(IsAccessDefinition<std::decay_t<decltype(access_definition)>>);
        return AccessDefinition{std::move(access_definition.buffer), std::move(access_definition.pattern), access_definition.Dofs, WriteConstant};
    };
    auto Accumulate = [](auto&& access_definition) {
        static_assert(IsAccessDefinition<std::decay_t<decltype(access_definition)>>);
        return AccessDefinition{std::move(access_definition.buffer), std::move(access_definition.pattern), access_definition.Dofs, AccumulateConstant};
    };
    auto ReadWrite = [](auto&& access_definition) {
        static_assert(IsAccessDefinition<std::decay_t<decltype(access_definition)>>);
        return AccessDefinition{std::move(access_definition.buffer), std::move(access_definition.pattern), access_definition.Dofs, ReadWriteConstant};
    };
    //! \}
} // namespace HPM

#endif