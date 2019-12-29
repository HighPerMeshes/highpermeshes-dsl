// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_ENTITIES_SIMPLEX_HPP
#define DSL_ENTITIES_SIMPLEX_HPP

#include <cstdint>
#include <algorithm>

#include <HighPerMeshes/dsl/entities/Geometry.hpp>
#include <HighPerMeshes/dsl/entities/Topology.hpp>

namespace HPM::entity
{
    using ::HPM::auxiliary::ConstexprFor;
    using ::HPM::dataType::Matrix;
    using ::HPM::math::Factorial;
    using ::HPM::math::GetCombination;

    //!
    //! \brief Implementation of the entity type simplex.
    //!
    //! This class contains implementations of the EntityTopology and EntityGeometry base classes.
    //!
    //! \tparam MeshT the mesh type
    //! \tparam CoordinateT the type of the nodes in the mesh
    //! \tparam EntityDimension the dimensionality of the entity (hierarchical definition)
    //! \tparam ParentEntityT the type of the parent entity
    //!
    template <typename MeshT_, typename CoordinateT, std::size_t EntityDimension, typename ParentEntityT_>
    class Simplex
    {
        // Only entities with a dimension lower than that of the cell type are allowed.
        static_assert(EntityDimension <= MeshT_::CellDimension, "error: entity dimension must be lower than or equal to the cell dimension");

        using Self = Simplex<MeshT_, CoordinateT, EntityDimension, ParentEntityT_>;

        public:
        // Template arguments.
        using MeshT = MeshT_;
        static constexpr std::size_t Dimension = EntityDimension;
        using ParentEntityT = ParentEntityT_;

        // Deduced types and constants.
        template <std::size_t Dimension>
        using EntityTypeWithDimension = Simplex<MeshT, CoordinateT, Dimension, Self>;
        using ScalarT = typename CoordinateT::ValueT;
        static constexpr std::size_t CellDimension = MeshT::CellDimension;
        static constexpr std::size_t WorldDimension = MeshT::WorldDimension;

        static constexpr bool IsFace = ((EntityDimension + 1) == CellDimension);

        //!
        //! \brief Implementation of the EntityTopology base class.
        //!
        class Topology : public EntityTopology<Simplex, MeshT, CoordinateT, EntityDimension, ParentEntityT>
        {
            using BaseTopology = EntityTopology<Simplex, MeshT, CoordinateT, EntityDimension, ParentEntityT>;

            // Friend declarations: allow other Entity types to access this type's members.
            template <template <typename, typename, std::size_t, typename> typename, typename, typename, std::size_t, typename>
            friend class EntityTopology;
            template <typename, typename, typename, std::size_t>
            friend class EntityGeometry;
            friend Simplex;

            Topology() = default;

            //!
            //! \brief Constructor.
            //!
            //! \param mesh a reference to the mesh
            //! \param local_index this index is always w.r.t. to an embedding super structure (if there is any)
            //! \param gloabl_index a unique index of this entity use for its identification among all entities in the mesh
            //! \param index_of_containing_cell a unique index of the cell that contains this entity
            //!
            Topology(const MeshT& mesh, const std::size_t local_index, const std::size_t index, const std::size_t index_of_containing_cell = MeshT::InvalidIndex)
                : BaseTopology(mesh, local_index, index, index_of_containing_cell)
            {
            }

            //!
            //! \brief Get the number of entites with a specified codimension.
            //!
            //! The calculation always considers an embedding dimension which is the dimension of
            //! an entity relative to the entity considered.
            //!
            //! \tparam Codimension the codimension of the entity considered
            //! \tparam EmbeddingDimension the dimension of the entity relative to the considered entity
            //! \return the number of entities
            //!
            template <std::size_t Codimension, std::size_t EmbeddingDimension = EntityDimension>
            static constexpr auto GetNumEntitiesWithCodimension()
            {
                static_assert(Codimension <= EmbeddingDimension, "error: codimension and embedding dimension are not compatible");

                // Vertices (entity dimensionality = 0)
                if constexpr (Codimension == EmbeddingDimension)
                {
                    return (EmbeddingDimension + 1);
                }
                // Entity itself.
                else if constexpr (Codimension == 0)
                {
                    return 1;
                }
                // Edges (entity dimensionality = 1)
                else if constexpr (Codimension == (EmbeddingDimension - 1))
                {
                    return (EmbeddingDimension * (EmbeddingDimension + 1)) / 2;
                }
                // All other cases: through recursion
                else
                {
                    return GetNumEntitiesWithCodimension<Codimension - 1, EmbeddingDimension - 1>() + GetNumEntitiesWithCodimension<Codimension, EmbeddingDimension - 1>();
                }
            }

            //!
            //! \brief Get a sub-array according to some selection value.
            //!
            //! The `selection` value determines a combination of `M` numbers out of `N` numbers
            //! and applies this combination to the input array to extract a subset of its values.
            //! This subset is sorted afterwards.
            //!
            //! \tparam M the length of the sub-array
            //! \tparam N the number of array elements
            //! \param in the input array
            //! \param selection the value which specifies the combination
            //! \return an array with elements taken from the input according to the selection
            //!
            template <std::size_t M, std::size_t N>
            static inline auto GetSubArray(const std::array<std::size_t, N>& in, const std::size_t selection) -> std::array<std::size_t, M>
            {
                // Determine the combination of M numbers out of N numbers and extract the corresponding values from the input array.
                auto out = ::HPM::auxiliary::GetSubArray(in, GetCombination<M, N>(selection));
                // Sort these values: avoid duplication of the same sub array with a different ordering of the elements.
                std::sort(out.begin(), out.end());

                return out;
            }

            //!
            //! \brief Get the number of entites with a specified dimension.
            //!
            //! The calculation always considers an embedding dimension which is the dimension of
            //! an entity relative to the entity considered.
            //!
            //! \tparam Dimension the dimension of the entity considered
            //! \tparam EmbeddingDimension the dimension of the entity relative to the considered entity
            //! \return the number of entities
            //!
            template <std::size_t Dimension, std::size_t EmbeddingDimension = EntityDimension>
            static constexpr auto GetNumEntitiesImplementation()
            {
                static_assert(Dimension <= EmbeddingDimension, "error: dimension and embedding dimension are not compatible");

                if constexpr (EmbeddingDimension == 3)
                {
                    if constexpr (Dimension == 0)
                        return 4;
                    else if constexpr (Dimension == 1)
                        return 6;
                    else if constexpr (Dimension == 2)
                        return 4;
                    else if constexpr (Dimension == 3)
                        return 1;
                }
                else
                {
                    return GetNumEntitiesWithCodimension<EmbeddingDimension - Dimension, EmbeddingDimension>();
                }
            }

            //!
            //! \brief Get the (global) indices of all entities with a specified dimension that are contained in this entity.
            //!
            //! Entities in the mesh are defined hierarchically. Cells consist of nodes and the corresponding mapping is given in the mesh file.
            //! All nodes have a unique ID. Cells also have a unique ID determined by their describing node indices. The latter can be sorted for a faster lookup.
            //! Entities have a sub-structure, defining entities of a lower dimension. These entities consist of subsets of the cell's node indices.
            //! Entities of higher dimension can share entities of a lower dimension, and the latter can be constructed all from the cells down to the edges.
            //! Using the subsets of the cell's node indices, all entities of the same dimension have unique IDs. These IDs are the (global) indices.
            //!
            //! The return type of this function is either a temporary or a reference to an existing array holding the indices.
            //!
            //! \tparam Dimension the dimension of the requested entities
            //! \return an array containing the (global) indices of the requested entities
            //!
            template <std::size_t Dimension>
            inline auto GetIndicesOfEntitiesWithDimensionImplementation() const
            {
                static_assert(Dimension <= EntityDimension, "error: dimension must be lower or equal to the entity dimension");

                if constexpr (Dimension == EntityDimension)
                {
                    return std::array<std::size_t, 1>{index};
                }
                else
                {
                    // Get this element's node indices and return all entities according to the combinations 'M out of N'.
                    const auto& node_indices = BaseTopology::GetNodeIndices();
                    const auto& entity_node_index_list = std::get<Dimension>(BaseTopology::MeshMemberAccess().EntityNodeIndexList());
                    std::array<std::size_t, GetNumEntitiesImplementation<Dimension>()> entities;

                    // Iterate over all sub-entities.
                    for (std::size_t local_index = 0; local_index < entities.size(); ++local_index)
                    {
                        // Get this entity's node indices.
                        const auto& entity_node_indices = GetSubArray<Dimension + 1>(node_indices, local_index);
                        // Look them up in the list of sets of entity node-indices of the requested dimension: binary search is definitely successful!
                        const auto it = std::lower_bound(entity_node_index_list.begin(), entity_node_index_list.end(), entity_node_indices);
                        // Get the (global) index.
                        entities[local_index] = std::distance(entity_node_index_list.begin(), it);
                    }

                    return entities;
                }
            }

            //!
            //! \brief Check for this entity being an element of the boundary in a geometric sense.
            //!
            //! A face is an element of the boundary if there is only one cell incident to it.
            //! A cell is an element of the boundary if any of its faces is an element of the boundary.
            //! All other entities are elements of the boundary of they belong to a face that is an element of the boundary.
            //!
            //! \return `true` if this entity is an element of the boundary, otherwise `false`
            //!
            inline auto IsElementOfBoundaryImplementation() const
            {
                if constexpr (IsFace)
                {
                    return (std::get<EntityDimension>(BaseTopology::MeshMemberAccess().EntityIncidenceList())[index].size() == 1);
                }
                else
                {
                    const auto& entity_boundary_list = std::get<EntityDimension>(BaseTopology::MeshMemberAccess().EntityBoundaryList());

                    return std::binary_search(entity_boundary_list.begin(), entity_boundary_list.end(), index);
                }
            }

            //!
            //! \brief Setup data structures in the mesh that hold topology information.
            //!
            //! \param mesh a reference to the mesh
            //!
            static auto SetupTopologyImplementation(MeshT& mesh)
            {
                static_assert(EntityDimension == CellDimension, "error: this entity is not a cell");

                // Get access to the mesh's members: this class is not a friend of Mesh, but EntityTopolgy (the base class) is.
                // Request a MemberAccessor from the base class. The MemberAccessor is a friend of the mesh.
                auto& mesh_entity_node_index_list = BaseTopology::MeshMemberAccess(mesh).EntityNodeIndexList();
                auto& mesh_entity_index_list = BaseTopology::MeshMemberAccess(mesh).EntityIndexList();
                auto& mesh_entity_incidence_list = BaseTopology::MeshMemberAccess(mesh).EntityIncidenceList();
                auto& mesh_entity_boundary_list = BaseTopology::MeshMemberAccess(mesh).EntityBoundaryList();
                auto& mesh_entity_neighbor_list = BaseTopology::MeshMemberAccess(mesh).EntityNeighborList();

                // (SUB-)ENTITIES within a cell:
                //
                // Create all sub-entities starting from the cell's node indices.
                //
                //   - cells consist of 'N' (=CellDimension+1) nodes
                //   - any sub-entity with dimension 'D' (0..(CellDimension-1)) consists of 'M=D+1' nodes
                //   - the number of sub-entities relative to the cell is 'factorial(N)/(factorial(M)*factorial(N-M))' (M out of N)
                //   - sub-entity 'I' corresponds to any of the combinations of the cell's node indices
                //
                //   - for each cell determine the node indices of sub-entity 'I' and sort them
                //   - insert the node indices of all sub-entities with dimension 'D' into the list 'mesh.entity_node_index_list[D]'
                //
                //  Result: a set of sorted lists of node indices of sub-entities with dimension 0..(CellDimension-1).
                for (std::size_t cell_index = 0; cell_index < mesh.GetNumEntities(); ++cell_index)
                {
                    // Node indices of this cell.
                    const auto& cell_node_indices = std::get<CellDimension>(mesh_entity_node_index_list)[cell_index];

                    // Construct all entities within this cell from the cell's node indices.
                    // Loop bounds: [1, CellDimension].
                    ConstexprFor<1, CellDimension + 1>([&cell_node_indices, &mesh_entity_node_index_list](const auto D) {
                        // Dimension of this (sub-)entity: (CellDimension - 1)..0.
                        constexpr std::size_t Dimension = CellDimension - D.value;
                        // Reference to the list of all (sub-)entities' node indices.
                        auto& entity_node_index_list = std::get<Dimension>(mesh_entity_node_index_list);
                        // The number of (sub-)entities with dimension 'Dimension' in this cell.
                        constexpr std::size_t NumEntities = GetNumEntitiesImplementation<Dimension, CellDimension>();

                        // Iterate over all (sub-)entities.
                        for (std::size_t entity_local_index = 0; entity_local_index < NumEntities; ++entity_local_index)
                        {
                            // Get this (sub-)entity's node indices.
                            const auto& entity_node_indices = GetSubArray<Dimension + 1>(cell_node_indices, entity_local_index);
                            // Look it up in the list of all (sub-)entities' node indices with the same dimension.
                            const auto it = std::lower_bound(entity_node_index_list.begin(), entity_node_index_list.end(), entity_node_indices);

                            if (it == entity_node_index_list.end() || (*it) != entity_node_indices)
                            {
                                // If the (sub-)entity cannot be found, add it: the resulting list is sorted.
                                entity_node_index_list.insert(it, entity_node_indices);
                            }
                        }
                    });
                }

                // Resize some fields / lists.
                ConstexprFor<CellDimension + 1>([&mesh, &mesh_entity_index_list, &mesh_entity_incidence_list, &mesh_entity_neighbor_list](const auto D) {
                    // For each cell, we need to store the indices of its sub-entities: resize the list in all dimensions to the number of cells.
                    std::get<D>(mesh_entity_index_list).resize(mesh.GetNumEntities());
                    // Use the number of (sub-)entites with dimension 'D' for the resizing.
                    std::get<D>(mesh_entity_incidence_list).resize(mesh.template GetNumEntities<D>());
                    std::get<D>(mesh_entity_neighbor_list).resize(mesh.template GetNumEntities<D>());
                });

                // (SUB-)ENTITIES within a cell (continued):
                //
                //   - given the set of sorted lists of node indices create all node indices of (sub-)entities with dimension 'D' as described above
                //     and look them up in the list 'mesh.entity_node_index_list[D]' (binary search via std::lower_bound() is always successful)
                //   - the position of the returned iterator defines the 'index' of the sub-entity with dimension 'D'
                //   - each (sub-)entity has a 'local_index' relative to the containing cell
                //   - add the 'index' to the list of entity-indices at position 'mesh.entity_index_list[D][cell_index][local_index]'
                //   - sub-entities with cell-dimension are the cells themselves, that is, 'mesh.entity_index_list[D][cell_index][0] = cell_index'
                //
                // Result: a list of indices of all sub-entities of dimension 0..CellDimension relative to their containing cell
                for (std::size_t cell_index = 0; cell_index < mesh.GetNumEntities(); ++cell_index)
                {
                    // Identical mapping for cells.
                    std::get<CellDimension>(mesh_entity_index_list)[cell_index][0] = cell_index;

                    // Get the node indices of this cell.
                    const auto& cell_node_indices = std::get<CellDimension>(mesh_entity_node_index_list)[cell_index];

                    // Construct all entities within this cell from the cell's node indices.
                    // Loop bounds: [1, CellDimension].
                    ConstexprFor<1, CellDimension + 1>([&cell_node_indices, &mesh_entity_node_index_list, &mesh_entity_index_list, cell_index](const auto D) {
                        // Dimension of this (sub-)entity: (CellDimension - 1)..0.
                        constexpr std::size_t Dimension = CellDimension - D;
                        // reference to the list of all (sub-)entities
                        const auto& entity_node_index_list = std::get<Dimension>(mesh_entity_node_index_list);
                        // The number of (sub-)entities with dimension 'Dimension' in this cell.
                        constexpr std::size_t NumEntities = GetNumEntitiesImplementation<Dimension, CellDimension>();
                        // Reference to the list of the indices of all (sub-)entities with dimension 'Dimension'.
                        auto& entity_indices = std::get<Dimension>(mesh_entity_index_list);

                        // Iterate over all (sub-)entities.
                        for (std::size_t entity_local_index = 0; entity_local_index < NumEntities; ++entity_local_index)
                        {
                            // Get this (sub-)entity's node indices.
                            const auto& entity_node_indices = GetSubArray<Dimension + 1>(cell_node_indices, entity_local_index);
                            // Look them up in the list of all (sub-)entities' node indices with the same dimension: use binary search!
                            const auto it = std::lower_bound(entity_node_index_list.begin(), entity_node_index_list.end(), entity_node_indices);
                            // std::distance(..) gives the index.
                            const std::size_t entity_index = std::distance(entity_node_index_list.begin(), it);
                            entity_indices[cell_index][entity_local_index] = entity_index;
                        }
                    });
                }

                // INCIDENT entities:
                //
                // For each entity with dimension 'D' (1..CellDimension) add the entity's index to
                // the list of incident entities of its sub-entities with dimension 'D-1'.
                //
                //   - for each entity with dimension 'D' create the node indices of its sub-entities with dimension 'D-1', look them up
                //     in 'mesh.entity_node_index_list[D-1]' and deduce the corresponding 'index'
                //   - add the index of the entity to its sub-entities' incidence lists 'mesh.entity_incidence_list[D-1][index]'
                //
                //  Result: a sorted list of incident entities for each entity
                for (std::size_t cell_index = 0; cell_index < mesh.GetNumEntities(); ++cell_index)
                {
                    // Construct all entities with dimension 1..CellDimension.
                    // Loop bounds: [0, CellDimension-1].
                    ConstexprFor<CellDimension>([&mesh_entity_node_index_list, &mesh_entity_index_list, &mesh_entity_incidence_list, cell_index](const auto D) {
                        // Dimension of this entity: CellDimension..1.
                        constexpr std::size_t Dimension = CellDimension - D;
                        // Reference to the list of the indices of all entities with dimension 'Dimension' within this cell.
                        const auto& entity_indices = std::get<Dimension>(mesh_entity_index_list)[cell_index];
                        // The number of (sub-)entities with dimension 'Dimension-1' relative to this entity (with dimension 'Dimension').
                        constexpr std::size_t NumSubEntities = GetNumEntitiesImplementation<Dimension - 1, Dimension>();
                        // Reference to the list of all node indices of entities with dimension 'Dimension-1'
                        const auto& sub_entity_node_index_list = std::get<Dimension - 1>(mesh_entity_node_index_list);

                        // Iterate over all entities with dimension 'Dimension' within this cell.
                        for (const std::size_t entity_index : entity_indices)
                        {
                            // Get the node indices of this entity.
                            const auto& entity_node_indices = std::get<Dimension>(mesh_entity_node_index_list)[entity_index];

                            // Iterator over all its sub-entities with dimension 'Dimension-1'.
                            for (std::size_t sub_entity_local_index = 0; sub_entity_local_index < NumSubEntities; ++sub_entity_local_index)
                            {
                                // Get this sub-entity's node indices.
                                const auto& sub_entity_node_indices = GetSubArray<Dimension>(entity_node_indices, sub_entity_local_index);
                                // Look them up in the list of all (sub-)entities' node indices with the same dimension: use binary search!
                                const auto it_sub_entity = std::lower_bound(sub_entity_node_index_list.begin(), sub_entity_node_index_list.end(), sub_entity_node_indices);
                                const std::size_t sub_entity_index = std::distance(sub_entity_node_index_list.begin(), it_sub_entity);
                                // Look up the entity index in the list of the (sub-)entity's incident entities.
                                auto& incident_entities = std::get<Dimension - 1>(mesh_entity_incidence_list)[sub_entity_index];
                                const auto it = std::lower_bound(incident_entities.begin(), incident_entities.end(), entity_index);

                                if (it == incident_entities.end() || (*it) != entity_index)
                                {
                                    // If the entity index cannot be found, add it: the resulting list is sorted.
                                    incident_entities.insert(it, entity_index);
                                }
                            }
                        }
                    });
                }

                // NEIGHBOR entities:
                //
                // For each entity with dimension 'D' (1..CellDimension) add the indices of all entities of the same dimension to
                // its list of neighbor entities if they share the same sub-entity with dimension 'D-1'.
                //
                //   - for each entity with dimension 'D' create the node indices of its sub-entities with dimension 'D-1', look them up
                //     in 'mesh.entity_node_index_list[D-1]' and deduce the corresponding 'index'
                //   - for each sub-entity iterate over all entities with dimension 'D' that are incident to it (use 'mesh.entity_incidence_list[D-1][index]')
                //   - add the index of those incident entities that are different from the entity considered to its neighbor list
                //   - nodes are handled separately
                //
                //  Result: a sorted list of neighbor entities for each entity
                for (std::size_t cell_index = 0; cell_index < mesh.GetNumEntities(); ++cell_index)
                {
                    // Construct all entities with dimension 1..CellDimension.
                    // Loop bounds: [0, CellDimension-1].
                    ConstexprFor<CellDimension>(
                        [&mesh_entity_node_index_list, &mesh_entity_index_list, &mesh_entity_incidence_list, &mesh_entity_neighbor_list, cell_index](const auto D) {
                            // Dimension of this entity: CellDimension..1.
                            constexpr std::size_t Dimension = CellDimension - D;
                            // Reference to the list of the indices of all entities with dimension 'Dimension' within this cell.
                            const auto& entity_indices = std::get<Dimension>(mesh_entity_index_list)[cell_index];
                            // The number of (sub-)entities with dimension 'Dimension-1' relative to this entity (with dimension 'Dimension').
                            constexpr std::size_t NumSubEntities = GetNumEntitiesImplementation<Dimension - 1, Dimension>();
                            // Reference to the list of all node indices of entities with dimension 'Dimension-1'.
                            const auto& sub_entity_node_index_list = std::get<Dimension - 1>(mesh_entity_node_index_list);

                            // Iterate over all entities with dimension 'Dimension' within this cell.
                            for (const std::size_t entity_index : entity_indices)
                            {
                                // Get the node indices of this entity.
                                const auto& entity_node_indices = std::get<Dimension>(mesh_entity_node_index_list)[entity_index];

                                // Iterate over all its sub-entities with dimension 'Dimension-1'.
                                for (std::size_t sub_entity_local_index = 0; sub_entity_local_index < NumSubEntities; ++sub_entity_local_index)
                                {
                                    // Get this sub-entity's node indices.
                                    const auto& sub_entity_node_indices = GetSubArray<Dimension>(entity_node_indices, sub_entity_local_index);
                                    // Look them up in the list of all (sub-)entities' node indices with the same dimension: use binary search!
                                    const auto it_sub_entity = std::lower_bound(sub_entity_node_index_list.begin(), sub_entity_node_index_list.end(), sub_entity_node_indices);
                                    const std::size_t sub_entity_index = std::distance(sub_entity_node_index_list.begin(), it_sub_entity);
                                    // Look up the entity index in the list of the (sub-)entity's incident entities.
                                    const auto& incident_entities = std::get<Dimension - 1>(mesh_entity_incidence_list)[sub_entity_index];
                                    // Look up the indices of all entities incident to this sub-entity in the entity's neighbor list.
                                    auto& neighbor_entities = std::get<Dimension>(mesh_entity_neighbor_list)[entity_index];

                                    for (const auto incident_entity_index : incident_entities)
                                    {
                                        if (incident_entity_index != entity_index)
                                        {
                                            const auto it = std::lower_bound(neighbor_entities.begin(), neighbor_entities.end(), incident_entity_index);

                                            if (it == neighbor_entities.end() || (*it) != incident_entity_index)
                                            {
                                                // If the entity index cannot be found, add it: the resulting list is sorted.
                                                neighbor_entities.insert(it, incident_entity_index);
                                            }
                                        }
                                    }
                                }
                            }
                        });
                }

                // Node neighbors.
                for (std::size_t node_index = 0; node_index < mesh.template GetNumEntities<0>(); ++node_index)
                {
                    // Iterate the nodes of all incident edges and add those to the neighbor list that differ from the considered node.
                    const auto& incident_edges = std::get<0>(mesh_entity_incidence_list)[node_index];
                    auto& neighbor_nodes = std::get<0>(mesh_entity_neighbor_list)[node_index];

                    for (const std::size_t incident_edge_index : incident_edges)
                    {
                        const auto& edge_node_indices = std::get<1>(mesh_entity_node_index_list)[incident_edge_index];

                        for (const std::size_t neighbor_node_index : edge_node_indices)
                        {
                            if (neighbor_node_index != node_index)
                            {
                                // We can add this node safely: it cannot be in the list already as each incident edge is considered just once,
                                // and each edge contributes a distinct node that differs from the considered node.
                                neighbor_nodes.push_back(neighbor_node_index);
                            }
                        }
                    }

                    // Sort the neighbor list.
                    std::sort(neighbor_nodes.begin(), neighbor_nodes.end());
                }

                // BOUNDARY entities:
                //
                // If the cell has dimension 1, all (sub-)entities are boundary entities, otherwise
                //   - a cell is an element of the boundary, if any of its faces is element of the boundary
                //   - a face is an element of the boundary, if only one cell is incident to it
                //   - an entity is an elements of the boundary, if its parent entity is an element of the boundary
                if constexpr (CellDimension == 1)
                {
                    // All (sub-)entities are boundary entities.
                    ConstexprFor<2>([&mesh_entity_node_index_list, &mesh_entity_boundary_list](const auto D) {
                        const std::size_t num_entities = std::get<D>(mesh_entity_node_index_list).size();
                        std::get<D>(mesh_entity_boundary_list).resize(num_entities);
                        for (std::size_t enitity_index = 0; enitity_index < num_entities; ++enitity_index)
                        {
                            std::get<D>(mesh_entity_boundary_list)[enitity_index] = enitity_index;
                        }
                    });
                }
                else
                {
                    constexpr std::size_t FaceDimension = CellDimension - 1;

                    for (std::size_t cell_index = 0; cell_index < mesh.GetNumEntities(); ++cell_index)
                    {
                        bool is_element_of_boundary = false;

                        // For each cell, iterate over all its faces.
                        for (const std::size_t face_index : std::get<FaceDimension>(mesh_entity_index_list)[cell_index])
                        {
                            // If this face has just one incident cell it is an element of the boundary.
                            if (std::get<FaceDimension>(mesh_entity_incidence_list)[face_index].size() == 1)
                            {
                                is_element_of_boundary = true;

                                // Add this face to the boundary-entity list if not already contained.
                                auto& boundary_entities = std::get<FaceDimension>(mesh_entity_boundary_list);
                                const auto it = std::lower_bound(boundary_entities.begin(), boundary_entities.end(), face_index);

                                if (it == boundary_entities.end() || (*it) != face_index)
                                {
                                    boundary_entities.insert(it, face_index);
                                }

                                // Add all entities this face is made up of to the boundary-entity list: deduce them from the face's node indices.
                                const auto& face_node_indices = std::get<FaceDimension>(mesh_entity_node_index_list)[face_index];

                                // Loop bounds: [1, CellDimension-1]
                                ConstexprFor<1, CellDimension>([&face_node_indices, &mesh_entity_node_index_list, &mesh_entity_boundary_list](const auto D) {
                                    // Dimension of this entity: (CellDimension-2)..0.
                                    constexpr std::size_t Dimension = FaceDimension - D;
                                    // Reference to the list of all node indices of entities with dimension 'Dimension'.
                                    const auto& entity_node_index_list = std::get<Dimension>(mesh_entity_node_index_list);
                                    // Reference to the boundary-entity list.
                                    auto& boundary_entities = std::get<Dimension>(mesh_entity_boundary_list);
                                    // Iterate over all entities that belong to this face.
                                    constexpr std::size_t NumSubEntities = GetNumEntitiesImplementation<Dimension, FaceDimension>();

                                    for (std::size_t entity_local_index = 0; entity_local_index < NumSubEntities; ++entity_local_index)
                                    {
                                        // Get this entity's node indices.
                                        const auto& entity_node_indices = GetSubArray<Dimension + 1>(face_node_indices, entity_local_index);
                                        // Look them up in the list of all entities' node indices with the same dimension: use binary search!
                                        const auto it_entity = std::lower_bound(entity_node_index_list.begin(), entity_node_index_list.end(), entity_node_indices);
                                        const std::size_t entity_index = std::distance(entity_node_index_list.begin(), it_entity);
                                        // Look up the entity index in the neighbor-entity list.
                                        const auto it = std::lower_bound(boundary_entities.begin(), boundary_entities.end(), entity_index);

                                        if (it == boundary_entities.end() || (*it) != entity_index)
                                        {
                                            // If the entity index cannot be found, add it: the resulting list is sorted.
                                            boundary_entities.insert(it, entity_index);
                                        }
                                    }
                                });
                            }
                        }

                        if (is_element_of_boundary)
                        {
                            // Add the cell to the boundary-entity list.
                            std::get<CellDimension>(mesh_entity_boundary_list).push_back(cell_index);
                        }
                    }
                }
            }

            using BaseTopology::index;
            using BaseTopology::local_index;
            using BaseTopology::mesh;
        };

        //!
        //! \brief Implementation of the EntityGeometry base class.
        //!
        class Geometry : public EntityGeometry<Simplex, MeshT, CoordinateT, EntityDimension>
        {
            using BaseGeometry = EntityGeometry<Simplex, MeshT, CoordinateT, EntityDimension>;

            // Deduced types.
            using ScalarT = typename CoordinateT::ValueT;
            static constexpr bool IsCell = Simplex::Topology::IsCell;
            static constexpr bool CreatedByCell = Simplex::Topology::CreatedByCell;
            static constexpr bool IsFace = Simplex::Topology::IsFace;
            static constexpr bool IsEdge = Simplex::Topology::IsEdge;
            static constexpr bool IsNode = Simplex::Topology::IsNode;

            // Friend declarations: allow other Entity types to access this type's members.
            friend BaseGeometry;
            friend Simplex;

            Geometry() = default;

            //!
            //! \brief Constructor.
            //!
            //! \param mesh a reference to the mesh
            //! \param topology a reference to the topology class associated with this entity
            //!
            Geometry(const MeshT& mesh, const Topology& topology) : BaseGeometry(mesh, topology) {}

            //!
            //! \brief Get the orientation of the normal vector.
            //!
            //! This implementation works for entities with dimension 2 only and if the world dimension is 3.
            //! Inwards: \f$-1\f$.
            //! Outwards: \f$+1\f$.
            //! If the cell dimenion is 2, the normal does not have an orientation and \f$+1\f$ is returned.
            //! The same applies to faces with dimension 2 that were created by cells.
            //!
            //! \return the orientation of the normal
            //!
            inline auto GetNormalOrientationImplementation() const
            {
                static_assert(WorldDimension == 3 && EntityDimension == 2, "error: implementation not available");

                if constexpr (IsFace && CreatedByCell)
                {
                    const std::size_t cell_index = topology.GetIndexOfContainingCell();
                    const std::size_t face_index = topology.GetLocalIndex();

                    return std::get<2>(BaseGeometry::MeshMemberAccess().NormalOrientations())[cell_index][face_index];
                }
                else
                {
                    return 1;
                }
            }

            //!
            //! \brief Get the normal coordinates of this entity.
            //!
            //! This implementation works for entities with dimension 2 only and if the world dimension is 3.
            //! If this entity is a face of a cell, this function returns a normal that points out ot the cell.
            //! If the cell dimension is 2, the normal does not have an orientation.
            //!
            //! \return the normal coordinates of this entity
            //!
            inline auto GetNormalImplementation() const
            {
                static_assert(WorldDimension == 3 && EntityDimension == 2, "error: implementation not available");

                const std::size_t entity_index = topology.GetIndex();
                const auto& normal = std::get<2>(BaseGeometry::MeshMemberAccess().Normals())[entity_index];

                return (normal * GetNormalOrientationImplementation());
            }

            //!
            //! \brief Get the unit-normal coordinates of this entity.
            //!
            //! This implementation works for entities with dimension 2 only and if the world dimension is 3.
            //! If this entity is a face of a cell, this function returns a normal that points out ot the cell.
            //! If the cell dimension is 2, the normal does not have an orientation.
            //!
            //! \return the normal coordinates of this entity
            //!
            inline auto GetUnitNormalImplementation() const { return Normalize(GetNormalImplementation()); }

            //!
            //! \brief Get the length of the normal vector.
            //!
            //! This implementation works for entities with dimension 2 only and if the world dimension is 3.
            //!
            //! \return the length of the normal vector
            //!
            inline auto GetNormalLengthImplementation() const { return GetNormalImplementation().Norm(); }

            //!
            //! \brief Get the volume of this entity.
            //!
            //! \return the volume of this entity
            //!
            inline auto GetVolumeImplementation() const
            {
                if constexpr (IsNode)
                {
                    return 0;
                }
                else if constexpr (IsEdge)
                {
                    // Volume = length of the edge.
                    const auto& nodes = topology.GetNodes();

                    return CoordinateT(nodes[1] - nodes[0]).Norm();
                }
                else if constexpr (IsFace && WorldDimension == 3)
                {
                    // For a triangle with nodes {A,B,C} it holds: Volume = |v(AB) x v(AC)| / 2.
                    const auto& nodes = topology.GetNodes();
                    return CrossProduct(nodes[1] - nodes[0], nodes[2] - nodes[0]).Norm() * 0.5;
                }
                else if constexpr (IsCell)
                {
                    // For the D-simplex it holds: Volume = det|position_vectors| / factorial(D)
                    const auto& nodes = topology.GetNodes();

                    // Determine position vectors (relative to nodes[0]).
                    std::array<CoordinateT, nodes.size()> position_vectors;

                    for (std::size_t i = 1; i < nodes.size(); ++i)
                    {
                        position_vectors[i - 1] = nodes[i] - nodes[0];
                    }

                    Matrix matrix(position_vectors);

                    return (std::abs(matrix.Determinant()) / Factorial(EntityDimension));
                }
            }

            //!
            //! \brief Get the jacobian matrix for this entity.
            //!
            //! This implementation works for entities with dimension 2 and 3.
            //!
            //! \return the jacobian matrix for this entity
            //!
            inline auto GetJacobianImplementation() const -> Matrix<ScalarT, EntityDimension, WorldDimension>
            {
                const auto& nodes = topology.GetNodes();

                if constexpr (EntityDimension == 3 && WorldDimension == 3)
                {
                    return { (nodes[1][0] - nodes[0][0]), (nodes[2][0] - nodes[0][0]), (nodes[3][0] - nodes[0][0]),
                             (nodes[1][1] - nodes[0][1]), (nodes[2][1] - nodes[0][1]), (nodes[3][1] - nodes[0][1]),
                             (nodes[1][2] - nodes[0][2]), (nodes[2][2] - nodes[0][2]), (nodes[3][2] - nodes[0][2])
                    };
                }
                else if constexpr (EntityDimension == 2 && WorldDimension == 2)
                {
                    return { (nodes[1][0] - nodes[0][0]), (nodes[2][0] - nodes[0][0]),
                             (nodes[1][1] - nodes[0][1]), (nodes[2][1] - nodes[0][1])
                    };
                }
                else if constexpr (EntityDimension == 2 && WorldDimension == 3)
                {
                    return { (nodes[1][0] - nodes[0][0]), (nodes[2][0] - nodes[0][0]),
                             (nodes[1][1] - nodes[0][1]), (nodes[2][1] - nodes[0][1]),
                             (nodes[1][2] - nodes[0][2]), (nodes[2][2] - nodes[0][2])
                    };
                }
                else
                {
                    static_assert(EntityDimension == 2 || EntityDimension == 3, "error: implementation not available");
                }
            }

            //!
            //! \brief Get the inverse of the jacobian matrix for this entity.
            //!
            //! This implementation works for entities with dimension 2 and 3.
            //!
            //! \return the inverse of the jacobian matrix for this entity
            //!
            inline auto GetInverseJacobianImplementation() const -> Matrix<ScalarT, EntityDimension, WorldDimension>
            {
                const auto& J = GetJacobianImplementation();
                // TODO: Error message if det(J) = 0 -> matrix is singular -> no inverse exists
                const ScalarT J_determinant = 1.0 / J.Determinant();

                if constexpr (EntityDimension == 3 && WorldDimension == 3)
                {
                    const ScalarT drdx = (J.yy * J.zz - J.zy * J.yz) * J_determinant;
                    const ScalarT dsdx = -(J.xy * J.zz - J.zy * J.xz) * J_determinant;
                    const ScalarT dtdx = (J.xy * J.yz - J.yy * J.xz) * J_determinant;
                    const ScalarT drdy = -(J.yx * J.zz - J.zx * J.yz) * J_determinant;
                    const ScalarT dsdy = (J.xx * J.zz - J.zx * J.xz) * J_determinant;
                    const ScalarT dtdy = -(J.xx * J.yz - J.yx * J.xz) * J_determinant;
                    const ScalarT drdz = (J.yx * J.zy - J.zx * J.yy) * J_determinant;
                    const ScalarT dsdz = -(J.xx * J.zy - J.zx * J.xy) * J_determinant;
                    const ScalarT dtdz = (J.xx * J.yy - J.yx * J.xy) * J_determinant;

                    return {drdx, dsdx, dtdx, drdy, dsdy, dtdy, drdz, dsdz, dtdz};
                }
                else if constexpr (EntityDimension == 2 && WorldDimension == 2)
                {
                    const ScalarT drdx =  (J.yy) * J_determinant;
                    const ScalarT dsdx = -(J.xy) * J_determinant;
                    const ScalarT drdy = -(J.yx) * J_determinant;
                    const ScalarT dsdy =  (J.xx) * J_determinant;

                    return {drdx, dsdx, drdy, dsdy};
                }
                else
                {
                    static_assert((EntityDimension == 3 && WorldDimension == 3)||(EntityDimension == 2 && WorldDimension == 2), "error: implementation not available");
                }
            }

            //!
            //! \brief Setup data structures in the mesh that hold geometry information.
            //!
            //! \param mesh a reference to the mesh
            //!
            static auto SetupGeometryImplementation(MeshT& mesh)
            {
                static_assert(EntityDimension == CellDimension, "error: this entity is not a cell");

                // NORMALS:
                //
                // Calculate normal vectors for faces of tetrahedra or triangles
                //   - cell has dimension 3: face normals (orientation is stored separately)
                //   - cell has dimension 2: entity normals (no orientation)
                if constexpr (WorldDimension == 3 && (EntityDimension == 2 || EntityDimension == 3))
                {
                    // Reference to the list of normals vectors.
                    auto& normals = std::get<2>(BaseGeometry::MeshMemberAccess(mesh).Normals());

                    if (normals.empty())
                    {
                        normals.resize(mesh.template GetNumEntities<2>());
                    }

                    // Iterator over all cells and calculate the face normals.
                    if constexpr (EntityDimension == 2)
                    {
                        for (const auto& cell : mesh.GetEntities())
                        {
                            const std::size_t cell_index = cell.GetTopology().GetIndex();
                            const auto& cell_nodes = cell.GetTopology().GetNodes();

                            normals[cell_index] = CrossProduct(cell_nodes[1] - cell_nodes[0], cell_nodes[2] - cell_nodes[0]);
                        }
                    }
                    else
                    {
                        for (const auto& cell : mesh.GetEntities())
                        {
                            const std::size_t cell_index = cell.GetTopology().GetIndex();
                            const auto& cell_nodes = cell.GetTopology().GetNodes();
                            const CoordinateT v[5] = {cell_nodes[1] - cell_nodes[0], cell_nodes[2] - cell_nodes[0], cell_nodes[3] - cell_nodes[0], cell_nodes[2] - cell_nodes[1], cell_nodes[3] - cell_nodes[1]};
                            const CoordinateT face_normal[4] = {CrossProduct(v[0], v[1]), CrossProduct(v[0], v[2]), CrossProduct(v[3], v[4]), CrossProduct(v[1], v[2])};
                            const auto& face_indices = std::get<2>(BaseGeometry::MeshMemberAccess(mesh).EntityIndexList())[cell_index];

                            for (std::size_t face_local_index = 0; face_local_index < 4; ++face_local_index)
                            {
                                if (normals[face_indices[face_local_index]] == CoordinateT{})
                                {
                                    normals[face_indices[face_local_index]] = face_normal[face_local_index];
                                }
                            }
                        }
                    }

                    // Determine face-normal orientations: works only for faces that are embedded into a cell.
                    if constexpr (EntityDimension == 3)
                    {
                        // Reference to the list of normal orientations.
                        auto& normal_orientations = std::get<2>(BaseGeometry::MeshMemberAccess(mesh).NormalOrientations());

                        if (normal_orientations.empty())
                        {
                            normal_orientations.resize(mesh.GetNumEntities());
                        }

                        for (const auto& cell : mesh.GetEntities())
                        {
                            for (const auto& face : cell.GetTopology().GetSubEntities())
                            {
                                // Find the cell-node that is not contained in the face (there is only one such node).
                                const std::size_t node_index = [&cell, &face]() {
                                    for (const std::size_t cell_node_index : cell.GetTopology().GetNodeIndices())
                                    {
                                        bool contained = false;
                                        for (const std::size_t face_node_index : face.GetTopology().GetNodeIndices())
                                        {
                                            contained |= (cell_node_index == face_node_index);
                                        }

                                        if (!contained)
                                            return cell_node_index;
                                    }

                                    return MeshT::InvalidIndex;
                                }();

                                // Get the vector from any face node to the node not contained in the face.
                                const auto& node = (BaseGeometry::MeshMemberAccess(mesh).Nodes())[node_index];
                                const auto& v = node - face.GetTopology().GetNodes()[0];
                                // If the scalar product of the face normal and 'v' is negative, the face normal points outwards.
                                const ScalarT orientation = ((normals[face.GetTopology().GetIndex()] * v) <= 0 ? 1 : -1);

                                normal_orientations[cell.GetTopology().GetIndex()][face.GetTopology().GetLocalIndex()] = orientation;
                            }
                        }
                    }
                }
            }

            using BaseGeometry::mesh;
            using BaseGeometry::topology;
        };

        private:
        Simplex() = default;

        public:
        //!
        //! \brief Constructor.
        //!
        //! \param mesh a reference to the mesh
        //! \param local_index this index is always w.r.t. to an embedding super structure (if there is any)
        //! \param index a unique index of this entity use for its identification among all entities in the mesh
        //! \param index_of_containing_cell a unique index of the cell that contains this entity
        //!
        Simplex(const MeshT& mesh, const std::size_t local_index, const std::size_t index, const std::size_t index_of_containing_cell = MeshT::InvalidIndex)
            : topology(mesh, local_index, index, index_of_containing_cell), geometry(mesh, topology)
        {
        }

        //!
        //! \brief Constructor.
        //!
        //! This constructor is a wrapper. It creates a simplex with local index and (global) index being equal.
        //!
        //! \param mesh a reference to the mesh
        //! \param index a unique index of this entity use for its identification among all entities in the mesh
        //!
        Simplex(const MeshT& mesh, const std::size_t index) : Simplex(mesh, index, index) {}

        //!
        //! \brief Get a reference to the Topology member of this simplex.
        //!
        //! \return a reference to the Topology member of this simplex
        //!
        inline auto GetTopology() const -> const Topology& { return topology; }

        //!
        //! \brief Get a reference to the Geometry member of this simplex.
        //!
        //! \return a reference to the Geometry member of this simplex
        //!
        inline auto GetGeometry() const -> const Geometry& { return geometry; }

        //!
        //! \brief Get a reference to the mesh.
        //!
        //! \return a reference to the mesh
        //!
        inline auto GetMesh() const -> const MeshT& { return topology.GetMesh(); }

        //!
        //! \brief Check for equality of two simplices.
        //!
        //! Two simplices are considered equal if they are topologically equal.
        //!
        //! \param other is another simplex to compare with
        //! \return `true` if this entity equals `other` toplogically, otherwise `false`
        //!
        inline auto operator==(const Simplex& other) const { return (topology == other.GetTopology()); }

        private:
        Topology topology;
        Geometry geometry;
    };
} // namespace HPM::entity

#endif
