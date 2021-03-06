// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_BUFFERS_BUFFERBASE_HPP
#define DSL_BUFFERS_BUFFERBASE_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>
#include <HighPerMeshes/dsl/data_access/Dof.hpp>

namespace HPM
{
    //! The base class for each buffer.
    //!
    //! We require to have access to the degrees of freedom for each buffer inside the runtime.
    template <typename MeshT>
    class BufferBase
    {
        //! \return the offset for a given entity_index
        template <std::size_t Dimension>
        auto GetDofOffset(const std::size_t entity_index) const
        {
            return ::HPM::dof::GetOffset<Dimension>(mesh, dofs, entity_index);
        }

        auto GetDofOffsets() const
        {
            std::array<std::size_t, MeshT::CellDimension + 2> offsets;

            ::HPM::auxiliary::ConstexprFor<0, MeshT::CellDimension + 2>([&offsets, this] (const auto Dimension) {
                offsets[Dimension] = ::HPM::dof::GetOffset<Dimension>(mesh, dofs, 0);
            });

            return offsets;
        }

      public:
        BufferBase(const MeshT& mesh, const std::array<std::size_t, MeshT::CellDimension + 2>& dofs) : mesh(mesh), dofs(dofs), offsets(GetDofOffsets()) {}

        template <std::size_t Dimension>
        auto GetDofIndices(const std::size_t entity_index = 0UL) const -> std::vector<std::size_t>
        {
            // Global dofs are the same for all entities.
            const std::size_t index = (Dimension == (MeshT::CellDimension + 1) ? 0UL : entity_index);
            const std::size_t offset = offsets[Dimension] + index * dofs.template At<Dimension>();
            std::vector<std::size_t> indices(dofs.template At<Dimension>());

            for (std::size_t i = 0; i < indices.size(); ++i)
            {
                indices[i] = offset + i;
            }

            return indices;
        }

        const auto& GetMesh() const { return mesh; }

        const auto& GetDofs() const { return dofs; }

      protected:
        const MeshT& mesh;
        const dataType::ConstArray<std::size_t, MeshT::CellDimension + 2> dofs;
        const std::array<std::size_t, MeshT::CellDimension + 2> offsets;
    };
} // namespace HPM

#endif