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
#include <HighPerMeshes/dsl/data_access/DofPartition.hpp>

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
        DistributedBuffer(const std::size_t L1_index, MeshT const& mesh, const DofT dofs = DofT{}, Allocator const allocator = Allocator{}) 
            : 
            Base(mesh, dofs.Get()), data(allocator)
        {
            //! For each entity in the local range go through all DoF-Dimensions and the corresponding indices and assign them a position in the local buffer space.
            std::size_t local_index = 0;

            ::HPM::auxiliary::ConstexprFor<0, MeshT::CellDimension + 2>([this, &mesh, &dofs, L1_index, &local_index](const auto Codimension) mutable 
            {
                constexpr std::size_t Dimension = (MeshT::CellDimension + 1) - Codimension;
                const std::size_t num_dofs = dofs.template At<Dimension>();

                if constexpr (Dimension == (MeshT::CellDimension + 1))
                {
                    const std::size_t offset = 0;

                    for (std::size_t dof = 0; dof < num_dofs; ++dof)
                    {
                        global_to_local_index.emplace(offset + dof, local_index++);
                    }

                    local_offsets[MeshT::CellDimension + 1] = 0;
                }
                else
                {
                    local_offsets[Dimension] = local_index;

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
                }
            });

            data.resize(global_to_local_index.size());
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

        const auto& GetDofs() const { return Base::GetDofs(); }

        //! \return Start position and size of the dofs partition in `data` for a given `dimension`.
        auto GetDofs(const std::size_t dimension) const -> DofPartition<const std::vector<T, Allocator>>
        {
            assert(dimension <= (MeshT::CellDimension + 1));

            // Global dofs and cell dofs.
            if (dimension >= MeshT::CellDimension)
            {
                return GetDofPartition(dimension);
            }
            // All other dofs.
            else 
            {
                // Data consistency cannot be assured, because of shared sub-entities!
                assert(dimension >= MeshT::CellDimension);

                // Return an empty partition.
                return {data, 0, 0, dimension};
            }
        }

        //! \return Start position and size of the dofs partition in `data` for a given `entity`.
        template <typename EntityT>
        const auto GetDofs(const EntityT& entity) const
        {
            const auto& dof_partition = GetDofPartitionImplementation(EntityT::Dimension);

            return dof_partition.At(entity);
        }

        //! \return Start position and size of the dofs partition in `data` for a given `dimension`.
        auto GetDofPartition(const std::size_t dimension) const -> DofPartition<const std::vector<T, Allocator>>
        {
            assert(dimension <= (MeshT::CellDimension + 1));

            // Global dofs.
            if (dimension == (MeshT::CellDimension + 1))
            {
                return {data, local_offsets.at(dimension), dofs.At(dimension), dofs.At(dimension), dimension};
            }
            // Node dofs: these are the last ones in `data`.
            else if (dimension == 0)
            {
                return {data, local_offsets.at(dimension), data.size() - local_offsets.at(dimension), dofs.At(dimension), dimension};
            }
            else
            {
                return {data, local_offsets.at(dimension), local_offsets.at(dimension - 1) - local_offsets.at(dimension), dofs.At(dimension), dimension};
            }
        }

        auto GetSize() { return data.size(); }

        T* GetData() { return data.data(); };

        const T* GetData() const { return data.data(); };

        auto begin() { return data.begin(); }

        auto begin() const { return data.begin(); }

        auto end() { return data.end(); }

        auto end() const { return data.end(); }

      protected:
        using Base::mesh;
        using Base::dofs;
        std::vector<T, Allocator> data;
        std::unordered_map<std::size_t, std::size_t> global_to_local_index; //! Mapping from the global index to the vector index
        std::array<std::size_t, MeshT::CellDimension + 2> local_offsets;
    };
} // namespace HPM

#endif