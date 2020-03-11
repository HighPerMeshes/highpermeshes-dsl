// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_MESHES_PARTITIONEDMESH_HPP
#define DSL_MESHES_PARTITIONEDMESH_HPP

#include <array>
#include <cstdint>
#include <set>
#include <tuple>
#include <vector>

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>
#include <HighPerMeshes/auxiliary/Reader.hpp>
#include <HighPerMeshes/drts/Runtime.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>
#include <HighPerMeshes/dsl/meshes/Partitioner.hpp>

namespace HPM::mesh
{
    //!
    //! \brief Partitioned mesh data type.
    //!
    //! \tparam CoordinateT the coordinate type used for the node (vertex) representation
    //! \tparam EntityTypeName the class type of the mesh entities
    //! \tparam CellDimension the dimensionality of the mesh entity type 'cell' (can be lower than the coordinate dimension)
    //!
    template <typename CoordinateT, template <typename, typename, std::size_t, typename> typename EntityTypeName, std::size_t CellDimension = CoordinateT::Dimension>
    class PartitionedMesh : public Mesh<CoordinateT, EntityTypeName, CellDimension>
    {
        using Self = PartitionedMesh<CoordinateT, EntityTypeName, CellDimension>;
        using MeshBase = Mesh<CoordinateT, EntityTypeName, CellDimension>;

        template <std::size_t Dimension>
        using EntityRange = typename MeshBase::template EntityRange<Dimension>;

        static constexpr std::size_t NumNodesPerCell = MeshBase::NumNodesPerCell;
        static constexpr std::size_t NumNodesPerFace = MeshBase::NumNodesPerFace;

        public:
        // Deduced types and constants.
        using Iterator = ::HPM::iterator::Range<std::size_t>;

        //!
        //! \brief Private Constructor
        //!
        template <typename Partitioner = SimplePartitioner>
        PartitionedMesh(std::vector<CoordinateT>&& nodes, std::vector<std::array<std::size_t, NumNodesPerCell>>&& cell_node_index_list, const std::pair<std::size_t, std::size_t> num_partitions = {1, 1}, std::size_t myL1Partition = 0,
                        Partitioner partitioner = Partitioner{})
            : PartitionedMesh(partitioner.template CreatePartitions<NumNodesPerFace>(std::move(nodes), std::move(cell_node_index_list), num_partitions), num_partitions, myL1Partition)
        {
        }

        private:
        //!
        //! \brief Constructor.
        //!
        //! This constructor initializes the mesh base class using nodes and elements,
        //! sets up the mapping between cells/nodes and their level-2 (L2) partitions,
        //! as well as the mapping between L2 partitions and entities of any dimension up to the cell dimension.
        //!
        //! \param tuple input data provided by the partitioner
        //! \param num_partitions the number of level-1 (L1) and L2 partitions to be created
        //!
        template <typename TupleT>
        PartitionedMesh(TupleT&& tuple, const std::pair<std::size_t, std::size_t>& num_partitions, std::size_t myL1Partition)
            : MeshBase(std::move(std::get<0>(tuple)), std::move(std::get<1>(tuple))), cell_to_L2P(std::move(std::get<2>(tuple))), node_to_L2P(std::move(std::get<3>(tuple))),
                L2P_to_cell_offset(std::move(std::get<4>(tuple))), L2P_to_node_offset(std::move(std::get<5>(tuple))), num_partitions(num_partitions), MyL1Partition(myL1Partition)
        {
            // Determine all L2 partitions assigned to this process.
            const std::size_t proc_id = MyL1Partition;
            const std::size_t num_procs = num_partitions.first;
            const std::size_t num_L2_partitions = GetNumL2Partitions();
            const std::size_t num_L2_partitions_per_proc = num_L2_partitions / num_procs;
            const std::size_t L2_begin = proc_id * num_L2_partitions_per_proc;
            const std::size_t L2_end = L2_begin + num_L2_partitions_per_proc;

            // Loop bounds: [0, CellDimension].
            ::HPM::auxiliary::ConstexprFor<CellDimension>([num_L2_partitions, L2_begin, L2_end, this](const auto Dimension) {
                // Resize to the total number of L2 partitions: we do not use index remapping, and thus need the whole index range.
                std::get<Dimension>(L2P_to_entity).resize(num_L2_partitions);

                // Iterate over all L2 partitions assigned to this process.
                for (std::size_t i_L2 = L2_begin; i_L2 < L2_end; ++i_L2)
                {
                    auto& list = std::get<Dimension>(L2P_to_entity)[i_L2];

                    // Iterate over all elements in this partition.
                    for (const auto& cell : L2PToEntity(i_L2))
                    {
                        const std::size_t cell_index = cell.GetTopology().GetIndex();

                        for (const auto& entity : cell.GetTopology().template GetEntities<Dimension>())
                        {
                            const std::size_t entity_index = entity.GetTopology().GetIndex();
                            const auto& it = std::lower_bound(list.begin(), list.end(), entity_index);

                            if (it == list.end() || (*it) != entity_index)
                            {
                                // Get indices of all cells that contain this entity.
                                std::vector<std::size_t> containing_cell_indices = entity.GetTopology().GetIndicesOfAllContainingCells();

                                // Remove all cells that do not have a lower index than the current cell.
                                containing_cell_indices.erase(std::remove_if(containing_cell_indices.begin(), containing_cell_indices.end(), [cell_index](const std::size_t& index) { return index >= cell_index; }),
                                                                containing_cell_indices.end());

                                // If the list is empty, this cell is the one with the lowest index and the entity belongs to its L2 partition.
                                if (containing_cell_indices.empty())
                                {
                                    list.insert(it, entity_index);
                                }
                            }
                        }
                    }
                }

                // Entity to L2 partition mapping.
                const std::size_t num_entities = std::get<Dimension>(MeshBase::entity_node_index_list).size();
                auto& list = std::get<Dimension>(entity_to_L2P);

                list.resize(num_entities);

                for (const auto& entity : MeshBase::template GetEntities<Dimension>(0, num_entities))
                {
                    // This entity belongs to the cell with the lowest (global) index: the requested indices are sorted already.
                    std::vector<std::size_t> containing_cell_indices = entity.GetTopology().GetIndicesOfAllContainingCells();
                    const std::size_t cell_index = containing_cell_indices.at(0);

                    // Get the L2 partition of this cell and store it.
                    list.at(entity.GetTopology().GetIndex()) = CellToL2P(cell_index);
                }
            });
        }

        public:
        //!
        //! \brief Create a partitioned mesh from a mesh file.
        //!
        //! This function extracts the nodes and the cell to node mappings from a mesh file,
        //! and creates the partitioned mesh using this data.
        //!
        //! \tparam Reader the class type of the mesh file reader
        //! \tparam Partitioner the type of the partitioner
        //! \param filename the name of the mesh file
        //! \param num_partitions the number of level-1 (L1) and level-2 (L2) partitions to be created
        //! \return a partitioned mesh object
        //!
        template <template <typename, typename> class Reader, typename Partitioner = SimplePartitioner>
        static PartitionedMesh CreateFromFile(const std::string& filename, const std::pair<std::size_t, std::size_t> num_partitions, std::size_t myL1Partition, Partitioner partitioner = Partitioner{})
        {
            Reader<CoordinateT, std::array<std::size_t, NumNodesPerCell>> reader;
            
            return CreateFromFile(filename, reader, num_partitions, myL1Partition, partitioner);
        }
        
        template <template <typename, typename> class Reader, typename Partitioner = SimplePartitioner>
        static PartitionedMesh CreateFromFile(const std::string& filename, Reader<CoordinateT, std::array<std::size_t, NumNodesPerCell>> reader, const std::pair<std::size_t, std::size_t> num_partitions, std::size_t myL1Partition, Partitioner partitioner = Partitioner{})
        {
            auto&& data = reader.ReadNodesAndElements(filename);

            return PartitionedMesh(std::move(std::get<0>(data)), std::move(std::get<1>(data)), num_partitions, myL1Partition, partitioner);
        }

        //!
        //! \brief Get the number of level-1 partitions.
        //!
        //! \return the number of level-1 partitions
        //!
        inline auto GetNumL1Partitions() const -> std::size_t { return std::get<0>(num_partitions); }

        //!
        //! \brief Get the number of level-2 (L2) partitions.
        //!
        //! This means the total number of L2 partitions, not the number of L2
        //! partitions per level-1 partition.
        //!
        //! \return the number of L2 partitions
        //!
        inline auto GetNumL2Partitions() const -> std::size_t { return std::get<0>(num_partitions) * std::get<1>(num_partitions); }

        //!
        //! \brief Get the containing level-1 (L1) partition of a level-2 (L2) partition.
        //!
        //! \param L2_index the index of the L2 partition
        //! \return the containing L1 partition
        //!
        inline auto L2PToL1P(const std::size_t L2_index) const { return (L2_index / std::get<1>(num_partitions)); }

        //!
        //! \brief Get all level-1 (L1) partitions of a level-2 (L2) partition
        //!
        //! \param L1_index the index of the L1 partition
        //! \return an iterator over all L2 partitions belonging to the specified L1 partition
        //!
        inline auto L1PToL2P(const std::size_t L1_index) const -> Iterator { return {L1_index * std::get<1>(num_partitions), (L1_index + 1) * std::get<1>(num_partitions)}; }

        //!
        //! \brief Get the level-2 (L2) partition of an entity.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \param index the (global) index of the entity
        //! \return the L2 partition this entity belongs to
        //!
        template <std::size_t Dimension>
        inline auto EntityToL2P(const std::size_t index) const
        {
            static_assert(Dimension <= CellDimension, "error: requested dimension is larger than the cell dimension");

            if constexpr (Dimension == CellDimension)
            {
                return cell_to_L2P.at(index);
            }
            else
            {
                return std::get<Dimension>(entity_to_L2P).at(index);
            }
        }

        //!
        //! \brief Get the level-2 (L2) partition of an entity.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \tparam ParentEntityT the type of the parent entity
        //! \param entity the entity
        //! \return the L2 partition this entity belongs to
        //!
        template <std::size_t Dimension, typename ParentEntityT>
        inline auto EntityToL2P(const EntityTypeName<MeshBase, CoordinateT, Dimension, ParentEntityT>& entity) const
        {
            return EntityToL2P<Dimension>(entity.GetTopology().GetIndex());
        }

        //!
        //! \brief Get the level-2 (L2) partition of a cell.
        //!
        //! \param index the (global) index of the cell
        //! \return the L2 partition this cell belongs to
        //!
        inline auto CellToL2P(const std::size_t index) const { return EntityToL2P<CellDimension>(index); }

        //!
        //! \brief Get the level-2 (L2) partition of a cell.
        //!
        //! \param cell a cell
        //! \return the L2 partition this cell belongs to
        //!
        template <typename CellT>
        inline auto CellToL2P(const CellT& cell) const
        {
            return EntityToL2P(cell);
        }

        //!
        //! \brief Get all entities of a given dimension within a level-2 (L2) partition.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \param L2_index the index of the L2 partition
        //! \return an iterator over all entities belonging to the specified L2 partition
        //!
        template <std::size_t Dimension = CellDimension>
        inline auto L2PToEntity(const std::size_t L2_index) const
        {
            static_assert(Dimension <= CellDimension, "error: requested dimension is larger than the cell dimension");

            if constexpr (Dimension == CellDimension)
            {
                return MeshBase::GetEntities(L2P_to_cell_offset.at(L2_index), L2P_to_cell_offset.at(L2_index + 1));
            }
            else
            {
                return MeshBase::template GetEntities<Dimension>(std::get<Dimension>(L2P_to_entity).at(L2_index));
            }
        }

        //!
        //! \brief Get all entities of a given dimension within a level-2 (L2) partition.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \param L2_index the index of the L2 partition
        //! \return an iterator over all entities belonging to the specified L2 partition
        //!
        template <typename RangeT>
        inline auto L2PToEntity(const RangeT& range, const std::size_t L2_index) const
        {
            return MeshBase::GetEntities(range, L2_index);
        }

        template <typename RangeT>
        inline auto GetEntities(const RangeT& range, const std::size_t L2_index) const
        {
            const auto& mesh = range.GetMesh();

            if (&mesh != this)
            {
                throw std::runtime_error("error: the range has not been created by this mesh");
            }

            return MeshBase::template GetEntities<RangeT::EntityDimension>(range.GetIndices(L2_index));
        }

        template <std::size_t Dimension = CellDimension>
        inline auto GetEntities(const std::vector<std::size_t>& indices) const
        {
            return MeshBase::template GetEntities<Dimension>(indices);
        }

        template <std::size_t Dimension = CellDimension>
        inline auto GetEntities(const std::size_t L2_index) const
        {
            static_assert(Dimension <= CellDimension, "error: requested dimension is larger than the cell dimension");

            if constexpr (Dimension == CellDimension)
            {
                return MeshBase::GetEntities(L2P_to_cell_offset.at(L2_index), L2P_to_cell_offset.at(L2_index + 1));
            }
            else
            {
                return MeshBase::template GetEntities<Dimension>(std::get<Dimension>(L2P_to_entity).at(L2_index));
            }
        }

        template <std::size_t Dimension = CellDimension>
        inline auto GetEntities() const
        {
            return MeshBase::template GetEntities<Dimension>();
        }

        //!
        //! \brief Determine the index of an entity from its node indices.
        //!
        //! This function searches for the node indices in the list of node indices of all entities with the specified dimension.
        //! If the node indices were found, the position (the index of the entity) is returned, otherwise an `InvalidIndex` is returned.
        //! This function uses binary search: the node indices must be sorted.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \param node_indices a set of indices describing the entity
        //! \return the index of the entity with the `node_indices` given to that function or an `InvalidIndex` if the `node_indices` were not found
        //!
        template <std::size_t Dimension>
        auto GetIndex(const std::array<std::size_t, MeshBase::template EntityT<Dimension>::Topology::template GetNumEntities<0>()>& node_indices) const -> std::size_t
        {
            // Use binary search to find the index of the entity with the given node indices.
            //
            // Cells are sorted only within the level-2 (L2) partitions: go through them one after the other and use binary search within each L2 partition.
            // All entities with lower dimension are sorted as usual.
            if constexpr (Dimension == CellDimension)
            {
                for (std::size_t i_L2 = 0; i_L2 < GetNumL2Partitions(); ++i_L2)
                {
                    const std::size_t i_start = L2P_to_cell_offset[i_L2];
                    const std::size_t i_end = L2P_to_cell_offset[i_L2 + 1];

                    if (i_start == i_end)
                        continue;

                    const auto it = std::lower_bound(std::get<Dimension>(MeshBase::entity_node_index_list).begin() + i_start, std::get<Dimension>(MeshBase::entity_node_index_list).begin() + i_end, node_indices);

                    if (it != (std::get<Dimension>(MeshBase::entity_node_index_list).begin() + i_end) && (*it) == node_indices)
                    {
                        return std::distance(std::get<Dimension>(MeshBase::entity_node_index_list).begin(), it);
                    }
                }
            }
            else
            {
                const auto it = std::lower_bound(std::get<Dimension>(MeshBase::entity_node_index_list).begin(), std::get<Dimension>(MeshBase::entity_node_index_list).end(), node_indices);

                if (it != std::get<Dimension>(MeshBase::entity_node_index_list).end() && (*it) == node_indices)
                {
                    return std::distance(std::get<Dimension>(MeshBase::entity_node_index_list).begin(), it);
                }
            }

            return MeshBase::InvalidIndex;
        }

        //!
        //! \brief Get an entity range.
        //!
        //! This function returns a `Range` type with entity indices according to the evaluation of a callable.
        //! The callable is evaluation for all entities of the specified dimension.
        //! If the callable evaluates to `true`, the (global) entity index is added to the index set.
        //! Otherwise, it is left out.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \tparam FuncT the type of the callable
        //! \param func a callable
        //! \return a `Range` type according to the entity dimension and the evaluation of the callable
        //!
        template <std::size_t Dimension, typename FuncT>
        inline auto GetEntityRange(FuncT func) const
        {
            static_assert(Dimension <= CellDimension, "error: requested dimension is larger than the cell dimension");

            // Determine all L2 partitions assigned to this process.
            const std::size_t proc_id = MyL1Partition;
            const std::size_t num_L2_partitions = GetNumL2Partitions();
            std::vector<std::vector<std::size_t>> entity_indices(num_L2_partitions);

            // Iterate over all L2 partitions assigned to this process.
            for (auto i_L2 : this->L1PToL2P(proc_id))
            {
                // Get an `EntityRange` over all entities within this L2 partition.
                if constexpr (Dimension == CellDimension)
                {
                    entity_indices[i_L2] = MeshBase::template GetEntityRange<Dimension>(func, L2P_to_cell_offset[i_L2], L2P_to_cell_offset[i_L2 + 1]).GetIndices();
                }
                else
                {
                    entity_indices[i_L2] = MeshBase::template GetEntityRange<Dimension>(func, std::get<Dimension>(L2P_to_entity)[i_L2]).GetIndices();
                }
            }

            return Range<Dimension, Self>(*this, std::move(entity_indices));
        }

        //!
        //! \brief Get an entity range.
        //!
        //! Wrapper: all entity indices are added to the index set.
        //!
        //! \return a `Range` type according to the entity dimension
        //!
        template <std::size_t Dimension>
        inline auto GetEntityRange() const
        {
            return GetEntityRange<Dimension>([](const auto&) { return true; });
        }

        private:
        std::vector<std::size_t> cell_to_L2P;
        std::vector<std::size_t> node_to_L2P;
        std::vector<std::size_t> L2P_to_cell_offset;
        std::vector<std::size_t> L2P_to_node_offset;
        std::array<std::vector<std::vector<std::size_t>>, CellDimension> L2P_to_entity;
        std::array<std::vector<std::size_t>, CellDimension> entity_to_L2P;
        const std::pair<std::size_t, std::size_t> num_partitions;
        const size_t MyL1Partition;
    };
} // namespace HPM::mesh

#include <HighPerMeshes/drts/data_flow/DataDependencyMaps.hpp>

#endif