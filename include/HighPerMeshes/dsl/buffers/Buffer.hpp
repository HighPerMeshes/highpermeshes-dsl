// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_BUFFERS_BUFFER_HPP
#define DSL_BUFFERS_BUFFER_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/buffers/BufferBase.hpp>

namespace HPM
{
    //!
    //! The class represents a vector where each entry represents a
    //! degree of freedom, which can be associated to a mesh entity.
    //!
    //! \tparam T The type of data stored.
    //! \tparam MeshT_ The type of mesh used.
    //! \tparam DofT_ The type of degrees of freedom used.
    //! \tparam Allocator The allocator for the underlying data structure.
    //!
    //! \note A constant buffer will not lazily allocate new data
    //!
    //! \see Dof.hpp
    //! \see Mesh.hpp
    //!
    template <typename T, typename MeshT_, typename DofT_, typename Allocator = std::allocator<T>>
    class Buffer : public BufferBase<MeshT_>
    {
        template <typename MeshT, typename DofT>
        inline auto GetTotalNumDofs(const MeshT& mesh, const DofT& dofs) const
        {
            return dof::GetOffset<0>(mesh, dofs, 0) + dofs.template At<0>() * mesh.template GetNumEntities<0>();
        }

      public:
        using ValueT = T;
        using MeshT = MeshT_;
        using DofT = DofT_;

        static_assert(DofT::Size() == MeshT::CellDimension + 1, "error: DofT has wrong size");

        Buffer(const MeshT& mesh, const DofT dofs = DofT{}, const Allocator& allocator = Allocator())
            : BufferBase<MeshT>(mesh, dofs.Get()), data(GetTotalNumDofs(mesh, dofs), allocator), mesh(mesh), dofs(dofs)
        {
        }

        //! \return Given an index, return the corresponding buffer entry
        //!
        //! \{
        auto operator[](std::size_t dofIdx) -> T& { return data[dofIdx]; }

        auto operator[](std::size_t dofIdx) const -> T const& { return data[dofIdx]; }
        //! \}

        //! \return Given a set of indices, GetRange returns the RandomAccessRange corresponding to these indices
        //! \see Iterator.hpp
        //! \{
        auto GetRange(std::set<std::size_t>&& indices) { return ::HPM::iterator::RandomAccessRange{data, std::move(indices)}; }

        auto GetRange(std::set<std::size_t>&& indices) const { return ::HPM::iterator::RandomAccessRange{data, std::move(indices)}; }

        auto GetRange(const std::set<std::size_t>& indices) { return ::HPM::iterator::RandomAccessRange{data, indices}; }

        auto GetRange(const std::set<std::size_t>& indices) const { return ::HPM::iterator::RandomAccessRange{data, indices}; }
        //! \}

        auto GetDofs() const -> const DofT& { return dofs; }

        std::size_t GetSize() const { return data.size(); }

        auto * Data() { return data.data(); }
        const auto * Data() const { return data.data(); }

      public:
        std::vector<T, Allocator> data;
        const MeshT& mesh;
        const DofT dofs;
    };

} // namespace HPM
#endif