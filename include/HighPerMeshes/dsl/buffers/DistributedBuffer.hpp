// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_BUFFERS_DISTRIBUTEDBUFFER_HPP
#define DSL_BUFFERS_DISTRIBUTEDBUFFER_HPP

#include <algorithm>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/buffers/BufferBase.hpp>

namespace HPM
{
    //!
    //! This class provides a buffer that lazily allocates space.
    //! It initially allocates enough for one L1 (global) partition.
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
    template <class T, typename MeshT_, typename DofT_, typename Allocator = std::allocator<T>>
    class DistributedBuffer : public BufferBase<MeshT_>
    {
        using Base = BufferBase<MeshT_>;

      public:
        using ValueT = T;
        using MeshT = MeshT_;
        using DofT = DofT_;

        //! \param L1_index The index of the global partition this buffer is defined on.
        DistributedBuffer(const std::size_t L1_index, MeshT const& mesh, const DofT dofs = DofT{}, Allocator const allocator = Allocator{}) : Base(mesh, dofs.Get()), data(allocator)
        {
            //! For each entity in the local range go through all DoF-Dimensions and the corresponding indices and assign them a position in the local buffer space.
            const std::size_t num_global_dofs = dofs.template At<MeshT::CellDimension + 1>();
            std::size_t local_index = dofs.template At<MeshT::CellDimension + 1>(); // global dofs first

            ::HPM::auxiliary::ConstexprFor<0, MeshT::CellDimension + 1>([this, &mesh, &dofs, L1_index, &local_index](const auto Dimension) mutable {
                const std::size_t num_dofs = dofs.template At<Dimension>();

                for (auto L2 : mesh.L1PToL2P(L1_index))
                {
                    for (const auto& entity : mesh.template L2PToEntity<Dimension>(L2))
                    {
                        const std::size_t offset = Base::offsets[Dimension] + num_dofs * entity.GetTopology().GetIndex();

                        for (std::size_t dof = 0; dof < num_dofs; ++dof)
                        {
                            global_to_local_index.emplace(offset + dof, local_index++);
                        }
                    }
                }
            });

            data.resize(num_global_dofs + global_to_local_index.size());
        }

        //! \return Given an index, return the corresponding buffer entry
        //!
        //! \note DistributedBuffer lazily allocates more data if the indices refer to buffer indices not previously existant in the buffer
        //!
        auto At(const std::size_t index) -> T&
        {
            if (global_to_local_index.find(index) == global_to_local_index.end())
            {
                global_to_local_index[index] = data.size();
                data.emplace_back();
            }

            return operator[](index);
        }

        auto At(const std::size_t index) const -> T const& { return operator[](index); }

        //! \return Given an index, return the corresponding buffer entry
        //!
        //! \note This will not lazily define new buffer indices and is intended for use in performance critical sections. Use `at` or `GetRange` for index checking and allocation.
        auto operator[](const std::size_t index) -> T& { return data[global_to_local_index[index]]; }

        auto operator[](const std::size_t index) const -> T const& { return data[global_to_local_index.at(index)]; }

        const auto& GetDofs() const { return dofs; }

        //! \return Given a set of indices, GetRange returns the RandomAccessRange corresponding to these indices
        //!
        //! \note DistributedBuffer lazily allocates more data if the indices refer to buffer indices not previously existant in the buffer
        //!
        //! \see Iterator.hpp
        auto GetRange(std::set<std::size_t>&& indices)
        {
            std::size_t count = data.size();
            std::set<std::size_t> adjusted_indices;

            std::transform(std::make_move_iterator(indices.begin()), std::make_move_iterator(indices.end()), std::inserter(adjusted_indices, adjusted_indices.begin()), [&count, this](auto index) {
                if (global_to_local_index.emplace(index, count).second)
                {
                    ++count;
                }

                return global_to_local_index[index];
            });

            if (count != data.size())
            {
                data.resize(count);
            }

            return ::HPM::iterator::RandomAccessRange{data, std::move(adjusted_indices)};
        }

        auto GetRange(const std::set<std::size_t>& indices)
        {
            std::size_t count = data.size();
            std::set<std::size_t> adjusted_indices;

            std::transform(indices.begin(), indices.end(), std::inserter(adjusted_indices, adjusted_indices.begin()), [&count, this](auto index) {
                if (global_to_local_index.emplace(index, count).second)
                {
                    ++count;
                }

                return global_to_local_index[index];
            });

            if (count != data.size())
            {
                data.resize(count);
            }

            return ::HPM::iterator::RandomAccessRange{data, std::move(adjusted_indices)};
        }

        auto GetRange(std::set<std::size_t>&& indices) const
        {
            std::set<std::size_t> adjusted_indices;

            std::transform(std::make_move_iterator(indices.begin()), std::make_move_iterator(indices.end()), std::inserter(adjusted_indices, adjusted_indices.begin()),
                [this](auto index) { return global_to_local_index.at(index); });

            return ::HPM::iterator::RandomAccessRange{data, std::move(adjusted_indices)};
        }

        auto GetRange(const std::set<std::size_t>& indices) const
        {
            std::set<std::size_t> adjusted_indices;

            std::transform(indices.begin(), indices.end(), std::inserter(adjusted_indices, adjusted_indices.begin()),
                [this](auto index) { return global_to_local_index.at(index); });

            return ::HPM::iterator::RandomAccessRange{data, std::move(adjusted_indices)};
        }

        auto GetSize() { return data.size(); }

        auto begin() const { return data.begin(); }

        auto end() const { return data.end(); }

      private:
        using Base::mesh;
        using Base::dofs;
        std::vector<T, Allocator> data;
        std::unordered_map<std::size_t, std::size_t> global_to_local_index; //! Mapping from the global index to the vector index
    };
} // namespace HPM

#endif