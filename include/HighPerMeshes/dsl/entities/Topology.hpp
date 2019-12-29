// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_ENTITIES_TOPOLOGY_HPP
#define DSL_ENTITIES_TOPOLOGY_HPP

#include <array>
#include <set>
#include <vector>

#include <HighPerMeshes/auxiliary/ConstexprIfElse.hpp>
#include <HighPerMeshes/auxiliary/ArrayOperations.hpp>
#include <HighPerMeshes/common/Iterator.hpp>

namespace HPM::entity
{
    using ::HPM::auxiliary::ConstexprIfElse;
    using ::HPM::auxiliary::TransformArray;

    //!
    //! \brief Topology base class.
    //!
    //! This implementation uses the type of the derived templated class in order to instantiate derived class types with differen template paramters
    //! which is needed for sub- and super-entity iteration.
    //! In short: entities can instantiate other entities with different dimension.
    //!
    //! \tparam EntityTypeName the template type of the mesh entities (CRTP)
    //! \tparam MeshT the mesh type
    //! \tparam CoordinateT the type of the nodes in the mesh
    //! \tparam EntityDimension the dimensionality of the entity (hierarchical definition)
    //! \tparam ParentEntityT the type of the parent entity
    //!
    template <template <typename, typename, std::size_t, typename> typename EntityTypeName, typename MeshT, typename CoordinateT, std::size_t EntityDimension, typename ParentEntityT>
    class EntityTopology
    {
        // Only entities with a dimension lower than that of the cell type are allowed.
        static_assert(EntityDimension <= MeshT::CellDimension, "error: entity dimension must be lower than or equal to the cell dimension");

        protected:
        // Deduced types and constants.
        static constexpr std::size_t CellDimension = MeshT::CellDimension;
        using ThisEntityT = EntityTypeName<MeshT, CoordinateT, EntityDimension, ParentEntityT>;
        static constexpr bool HasParentEntity = !(std::is_same_v<typename MeshT::NullT, ParentEntityT>);
        // If this entity does not have a parent entity, it is the parent itself.
        static constexpr std::size_t ParentEntityDimension = std::conditional_t<HasParentEntity, ParentEntityT, ThisEntityT>::Dimension;
        // Type of an entity of a certain dimensionality w.r.t. this entity.
        template <std::size_t Dimension>
        using EntityT = std::conditional_t<(Dimension > EntityDimension), typename MeshT::template EntityT<Dimension>,                               // has `MeshT::NullT` as the parent type
                                                            std::conditional_t<(Dimension == EntityDimension), ThisEntityT,                              // take the type of this entity
                                                                                EntityTypeName<MeshT, CoordinateT, Dimension, ThisEntityT>>>;             // has this type as the parent type
        template <typename ThisEntityT>
        using IndexedEntityRange = ::HPM::iterator::IndexedEntityRange<ThisEntityT, MeshT>;
        using Topology = typename ThisEntityT::Topology;
        static constexpr bool IsCell = (EntityDimension == CellDimension);
        static constexpr bool ParentEntityIsCell = (HasParentEntity && ParentEntityDimension == CellDimension);
        static constexpr bool CreatedByCell = (ParentEntityIsCell || (ParentEntityDimension > EntityDimension && std::conditional_t<HasParentEntity, ParentEntityT, ThisEntityT>::Topology::CreatedByCell));
        static constexpr bool IsFace = ((EntityDimension + 1) == CellDimension);
        static constexpr bool IsEdge = (EntityDimension == 1);
        static constexpr bool IsNode = (EntityDimension == 0);

        EntityTopology() = default;

        //!
        //! \brief Constructor.
        //!
        //! Each entity has a local index and a (global) index. If the entity type is a cell, no embedding super-structure exists and local and (global) index must be equal.
        //! The same applies to all cases where entities of any dimension are created directly through `MeshT::GetEntities(..)`.
        //! Only if entities are created by other entities, the local index reflects the embedding sub-/super-structure.
        //!
        //! Similarly, only entities created by other higher-dimensional entities can have a unique containing cell if either the creating entity is a cell or has been created
        //! by a higher-dimensional entity that has a unique containing cell. In all other cases, the relation between a particular entity and the containing cell is ambiguous.
        //!
        //! If this entity is a cell, the index of the containing cell is always equal to its (global) index.
        //!
        //! \param mesh a reference to the mesh
        //! \param local_index this index is always w.r.t. to an embedding super structure (if there is any)
        //! \param index a unique index of this entity use for its identification among all entities in the mesh
        //! \param index_of_containing_cell a unique index of the cell that contains this entity
        //!
        EntityTopology(const MeshT& mesh, const std::size_t local_index, const std::size_t index, const std::size_t index_of_containing_cell = MeshT::InvalidIndex)
            : mesh(mesh), local_index(local_index), index(index), index_of_containing_cell(ConstexprIfElse<IsCell>(index, index_of_containing_cell))
        {
        }

        //!
        //! \brief Get access to the mesh's member.
        //!
        //! This class is friend with the mesh, but its derived (implementing) classes are not.
        //! A friend declaration in the mesh for the derived classes shadows the `EntityTypeName` template paramter of the mesh, and hence is not allowed.
        //! Instead the mesh contains a `MemberAccessor` type which is friend with the mesh and can only be instantiated by `EntityTopology` and `EntityGeometry` class.
        //! This accessor holds a reference to the mesh and provides access to the mesh's members through getters.
        //! The accessor can be used by the derived classes to access these members.
        //!
        //! \return a mesh accessor object
        //!
        inline auto MeshMemberAccess() const
        {
            using MemberAccessor = typename MeshT::template MemberAccessor<const MeshT>;

            return MemberAccessor(mesh);
        }

        //!
        //! \brief Get access to the mesh's member.
        //!
        //! This class is friend with the mesh, but its derived (implementing) classes are not.
        //! A friend declaration in the mesh for the derived classes shadows the `EntityTypeName` template paramter of the mesh, and hence is not allowed.
        //! Instead the mesh contains a `MemberAccessor` type which is friend with the mesh and can only be instantiated by `EntityTopology` and `EntityGeometry` class.
        //! This accessor holds a reference to the mesh and provides access to the mesh's members through getters.
        //! The accessor can be used by the derived classes to access these members.
        //!
        //! \param mesh a reference to the mesh
        //! \return a mesh accessor object
        //!
        static inline auto MeshMemberAccess(MeshT& mesh)
        {
            using MemberAccessor = typename MeshT::template MemberAccessor<MeshT>;
            
            return MemberAccessor(mesh);
        }

        public:
        //!
        //! \brief Get a mesh reference.
        //!
        //! \return a const reference to the mesh
        //!
        inline const auto& GetMesh() const { return mesh; }

        //!
        //! \brief Get the local index of this entity.
        //!
        //! \return the local index of this entity
        //!
        inline auto GetLocalIndex() const { return local_index; }

        //!
        //! \brief Get the (global) index of this entity.
        //!
        //! \return the (global) index of this entity
        //!
        inline auto GetIndex() const { return index; }

        //!
        //! \brief Get this entity's node indices.
        //!
        //! Node indices are sorted entity-wise within any dimension up to the cell dimension.
        //! There is a 1:1 mapping between the (global) index of the entity and its node indices.
        //!
        //! \return the (global) indices of the nodes this entity consists of
        //!
        inline const auto& GetNodeIndices() const { return std::get<EntityDimension>(mesh.entity_node_index_list)[index]; }

        //!
        //! \brief Get the index of the cell that contains this entity.
        //!
        //! This function can only be called for entities that are cells or have been created by cells.
        //!
        //! \return the index of the cell that contains this entity
        //!
        inline auto GetIndexOfContainingCell() const
        {
            static_assert(CreatedByCell, "error: this entity is neither a cell nor has it been created by a cell");

            return index_of_containing_cell;
        }

        //!
        //! \brief Get the indices of all cells that contain this entity.
        //!
        //! If this entity is a cell, return a vector with the (global) index of this entity as its content.
        //! Otherwise, start from this entity's (global) index and iterate over all higher dimensional entities that are incident to it
        //! until the cell dimension is reached.
        //! The output of each iteration becomes the input of the next iteration.
        //! In order to count incident entities just once, this implementation uses an `std::set` data structure to hold the indices.
        //! At the end of the iteration, the content of the set is copied to a vector container which then is returned by this function.
        //!
        //! \return a vector container holding the indices of all cells that contain this entity
        //!
        inline auto GetIndicesOfAllContainingCells() const -> std::vector<std::size_t>
        {
            std::set<std::size_t> indices;
            indices.insert(index);

            if constexpr (!IsCell)
            {
                for (std::size_t d = EntityDimension; d < CellDimension; ++d)
                {
                    const auto& entity_incidence_list = mesh.entity_incidence_list[d];
                    std::set<std::size_t> tmp;

                    for (std::size_t index : indices)
                    {
                        const auto& incident_entities = entity_incidence_list[index];

                        tmp.insert(incident_entities.begin(), incident_entities.end());
                    }

                    indices.swap(tmp);
                }
            }

            return {indices.begin(), indices.end()};
        }

        //!
        //! \brief Get the index of the neighboring cell relative to this face.
        //!
        //! There is at most one neighboring cell.
        //! If there is no neighboring cell (e.g. at the boundary), the containing cell is considered the neighboring cell.
        //!
        //! This function can be called only for faces that have been created by cells.
        //!
        //! \return the index of the neighboring cell
        //!
        inline auto GetIndexOfNeighboringCell() const
        {
            static_assert(IsFace, "error: this is not a face entity");
            static_assert(ParentEntityIsCell, "error: this entity does not have a cell as a parent");

            const std::size_t cell_index = GetIndexOfContainingCell();
        
#if defined(MESH_VALUE_LOOKUP)
            const std::size_t face_index = local_index;
            return mesh.lookup_face_neighboring_cell_mapping[cell_index][face_index];
#else
            // Iterate over all incident cells and return the index of the cell that is not the containing cell.
            for (const auto& incident_cell : GetIncidentEntities())
            {
                const std::size_t incident_cell_index = incident_cell.GetTopology().GetIndex();

                if (incident_cell_index != cell_index)
                {
                    return incident_cell_index;
                }
            }

            return cell_index;
#endif
        }

        //!
        //! \brief Get the indices of all entities that are neighbors of this entity.
        //!
        //! \return a vector container holding the indices of all neighboring entities
        //!
        inline auto GetIndicesOfNeighboringEntities() const -> const std::vector<std::size_t>& { return mesh.entity_neighbor_list[EntityDimension][index]; }

        //!
        //! \brief Get the local index of the neighboring face relative to this face.
        //!
        //! There is at most one neighboring cell and face.
        //! If there is no neighboring cell (e.g. at the boundary), this face is considered the neighboring face.
        //!
        //! This function can be called only for faces that have been created by cells.
        //!
        //! \return the local index of the neighboring face
        //!
        inline auto GetLocalIndexOfNeighboringFace() const
        {
            static_assert(IsFace, "error: this is not a face entity");
            static_assert(ParentEntityIsCell, "error: this entity does not have a mesh entity as a parent");

            const std::size_t cell_index = GetIndexOfContainingCell();
            const std::size_t face_index = local_index;

#if defined(MESH_VALUE_LOOKUP)
            return mesh.lookup_face_neighboring_face_mapping[cell_index][face_index];
#else
            const auto& neighboring_cell = GetNeighboringCell();

            if (neighboring_cell.GetTopology().GetIndex() != cell_index)
            {
                for (const auto& neighboring_face : neighboring_cell.GetTopology().GetSubEntities())
                {
                    if ((*this) == neighboring_face)
                    {
                        return neighboring_face.GetTopology().GetLocalIndex();
                    }
                }
            }

            return face_index;
#endif
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
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \tparam Dimension the dimension of the requested entities
        //! \return an array containing the (global) indices of the requested entities
        //!
        template <std::size_t Dimension>
        inline auto GetIndicesOfEntitiesWithDimension() const
        {
            static_assert(Dimension <= EntityDimension, "error: dimension must be lower or equal to the entity dimension");

            // If this entity has the same dimension as the requested entity, return this entity's (global) index.
            if constexpr (Dimension == EntityDimension)
            {
                return std::array<std::size_t, 1>{index};
            }
            // If this entity is a cell, all sub-entity indices are stored in the entity_index_list.
            else if constexpr (IsCell)
            {
                return std::get<Dimension>(mesh.entity_index_list)[index];
            }
            // Otherwise, deduce the requested entity indices.
            else
            {
                // The requested entity type is a node.
                if constexpr (Dimension == 0)
                {
                    // return this entity's node indices.
                    return GetNodeIndices();
                }
                // IMPLEMENTATION SPECIFIC: CRTP.
                else
                {
                    // Get all-sub entities with the requested dimension.
                    return static_cast<const Topology&>(*this).template GetIndicesOfEntitiesWithDimensionImplementation<Dimension>();
                }
            }
        }

        //!
        //! \brief Get this entity's nodes.
        //!
        //! The nodes of each entity are collected by accessing the node coordinates according to the (global) node indices of this entity.
        //!
        //! \return the nodes of this entity
        //!
        inline auto GetNodes() const
        {
            return TransformArray(GetNodeIndices(), [this](const std::size_t index) { return mesh.nodes[index]; });
        }

        inline auto GetVertices() const
        {
            return GetNodes();
        }

        //!
        //! \brief Get the containing cell.
        //!
        //! The type of the containing cell is `MeshT::ThisEntityT`.
        //! Local and (global) index of the cell are equal in this case.
        //! The containing cell is the cell itself.
        //!
        //! \return the containing cell
        //!
        inline auto GetContainingCell() const -> EntityT<MeshT::CellDimension> { return {mesh, GetIndexOfContainingCell()}; }

        //!
        //! \brief Get an iterator-range over all cells that contain this entity.
        //!
        //! The type of the containing cells is `MeshT::ThisEntityT`.
        //! Local and (global) index of the cells are equal in this case.
        //! The containing cells are the cells themselve.
        //!
        //! \return an iterator range over the containing cells
        //!
        inline auto GetAllContainingCells() const -> IndexedEntityRange<EntityT<MeshT::CellDimension>> { return {mesh, GetIndicesOfAllContainingCells()}; }

        //!
        //! \brief Get the neighboring cell of this face.
        //!
        //! The type of the neighboring cell is `MeshT::ThisEntityT`.
        //! Local and (global) index of the cell are equal in this case.
        //! The containing cell is the cell itself.
        //!
        //! This function can be called only for faces that have been created by cells.
        //!
        //! \return the neighboring cell
        //!
        inline auto GetNeighboringCell() const -> EntityT<MeshT::CellDimension>
        {
            static_assert(IsFace, "error: this is not a face entity");
            static_assert(ParentEntityIsCell, "error: this entity does not have a mesh entity as a parent");

            return {mesh, GetIndexOfNeighboringCell()};
        }

        //!
        //! \brief Get an iterator range over all entities that are neighbors of this entity.
        //!
        //! \return an iterator range over all neighboring entities
        //!
        inline auto GetNeighboringEntities() const -> IndexedEntityRange<EntityT<EntityDimension>> { return {mesh, GetIndicesOfNeighboringEntities()}; }

        //!
        //! \brief Get an iterator range over all entities with a specified dimension that are contained in this entity.
        //!
        //! All entities with a lower dimension inherit the index of the containing cell from this entity.
        //!
        //! \tparam Dimension the dimension of the requested entities
        //! \return an iterator range over the requested entities
        //!
        template <std::size_t Dimension>
        inline auto GetEntities() const -> IndexedEntityRange<EntityT<Dimension>>
        {
            static_assert(Dimension <= EntityDimension, "error: dimension must be lower or equal to the entity dimension");

            const auto& element_indices = GetIndicesOfEntitiesWithDimension<Dimension>();

            return {mesh, {element_indices.begin(), element_indices.end()}, index_of_containing_cell};
        }

        //!
        //! \brief Get an iterator range over this entity's sub-entities.
        //!
        //! All sub-entities inherit the index of the containing cell from this entity.
        //!
        //! \return an iterator range over the sub-entities
        //!
        inline auto GetSubEntities() const
        {
            if constexpr (IsNode)
            {
                return IndexedEntityRange<EntityT<0>>(mesh, {});
            }
            else
            {
                return GetEntities<EntityDimension - 1>();
            }
        }

        //!
        //! \brief Get an iterator range over all entities that are incident to this entity.
        //!
        //! Part of the incident entities share the same containing cell with this entity.
        //! However, there can be incident entities that belong to a neighboring cell.
        //! We thus cannot propagate the index of the containing cell of this entity to all incident entities.
        //!
        //! \return an iterator range over all incident entities
        //!
        inline auto GetIncidentEntities() const -> IndexedEntityRange<EntityT<EntityDimension + 1>>
        {
            if constexpr (IsCell)
            {
                return {mesh, {}};
            }
            else
            {
                return {mesh, mesh.entity_incidence_list[EntityDimension][index]};
            }
        }

        //!
        //! \brief Get the number of cells that contain this entity.
        //!
        //! \return the number of containing cells
        //!
        inline auto GetNumContainingCells() const { return GetIndicesOfAllContainingCells().size(); }

        //!
        //! \brief Get the number of neighboring entities.
        //!
        //! \return the number of neighboring entities
        //!
        inline auto GetNumNeighboringEntities() const { return mesh.entity_neighbor_list[EntityDimension][index].size(); }

        //!
        //! \brief Get the number of incident entities.
        //!
        //! Incident entities are those that contain this entity and have a dimension which is higher by 1.
        //! If this entity has the dimension of the cell type, there are no incident elements and 0 is returned.
        //!
        //! \return the number of incident entities
        //!
        inline auto GetNumIncidentEntities() const
        {
            if constexpr (IsCell)
            {
                return 0;
            }
            else
            {
                return mesh.entity_incidence_list[EntityDimension][index].size();
            }
        }

        //!
        //! \brief Get the number of entities contained in this entitiy and with a lower dimension.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the number of entities with a lower dimension
        //!
        template <std::size_t Dimension>
        static constexpr auto GetNumEntities()
        {
            if constexpr (Dimension <= EntityDimension)
            {
                return Topology::template GetNumEntitiesImplementation<Dimension>();
            }
            else
            {
                return 0;
            }
        }

        //!
        //! \brief Check for whether a this entity has neighboring entities or not.
        //!
        //! \return `true` if this entity has neighboring entities, otherwise `false`
        //!
        inline auto HasNeighboringEntities() const { return (GetNumNeighboringEntities() != 0); }

        //!
        //! \brief Check for whether a this face has a neighboring cell.
        //!
        //! This function can be called only for faces that have been created by cells.
        //!
        //! \return `true` if this face has a neighboring cell, otherwise `false`
        //!
        inline auto HasNeighboringCell() const
        {
            static_assert(IsFace, "error: this is not a face entity");
            static_assert(ParentEntityIsCell, "error: this entity does not have a cell as a parent");

#if defined(MESH_VALUE_LOOKUP)
            const std::size_t cell_index = GetIndexOfContainingCell();
            const std::size_t face_index = local_index;

            return (mesh.lookup_face_neighboring_cell_mapping[cell_index][face_index] != cell_index);
#else
            return (GetNumIncidentEntities() > 1);
#endif
        }

        //!
        //! \brief Check for this entity being an element of the boundary in a geometric sense.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return `true` if this entity is an element of the boundary, otherwise `false`
        //!
        inline auto IsElementOfBoundary() const { return static_cast<const Topology&>(*this).IsElementOfBoundaryImplementation(); }

        //!
        //! \brief Get the boundary condition for this entity.
        //!
        //! This function can be called only for faces.
        //!
        //! \return the boundary condition
        //!
        inline auto GetBoundaryCondition() const
        {
            static_assert(IsFace, "error: this is not a face entity");

            using ReturnType = typename std::remove_reference_t<decltype(mesh.boundary_conditions[EntityDimension])>::value_type::second_type;

            if constexpr (IsFace)
            {
                if (IsElementOfBoundary())
                {
                    const auto& boundary_conditions = mesh.boundary_conditions[EntityDimension];
                    const auto it = std::lower_bound(boundary_conditions.begin(), boundary_conditions.end(), index, [](const auto& entity, const auto& reference) { return (entity.first < reference); });

                    if (it != boundary_conditions.end() && it->first == index)
                    {
                        return it->second;
                    }
                }
            }

            return static_cast<ReturnType>(0);
        }

        //!
        //! \brief Check for equality of two entities.
        //!
        //! Two entities are considered equal if their node indices are equal.
        //!
        //! \param other_topology is another entity topology to compare with
        //! \return `true` if this entity equals `other_topology`, otherwise `false`
        //!
        inline auto operator==(const EntityTopology& other_topology) const { return (GetNodeIndices() == other_topology.GetNodeIndices()); }

        //!
        //! \brief Check for equality of two entities.
        //!
        //! Two entities are considered equal if they are topologically equal.
        //!
        //! \param other is another entity to compare with
        //! \return `true` if this entity equals `other`, otherwise `false`
        //!
        inline auto operator==(const ThisEntityT& other) const { return ((*this) == other.GetTopology()); }

        //!
        //! \brief Setup data structures in the mesh that hold topology information.
        //!
        //! \param mesh a reference to the mesh
        //!
        static auto SetupTopology(MeshT& mesh)
        {
            // Setup data structures in the mesh.
            Topology::SetupTopologyImplementation(mesh);

#if defined(MESH_VALUE_LOOKUP)
            // Pre-calculate some geometry information.
            auto& lookup_face_neighboring_cell_mapping = mesh.lookup_face_neighboring_cell_mapping;
            auto& lookup_face_neighboring_face_mapping = mesh.lookup_face_neighboring_face_mapping;
            const std::size_t num_cells = mesh.GetNumEntities();

            if (lookup_face_neighboring_cell_mapping.empty())
            {
                lookup_face_neighboring_cell_mapping.resize(num_cells);
            }

            if (lookup_face_neighboring_face_mapping.empty())
            {
                lookup_face_neighboring_face_mapping.resize(num_cells);
            }

            for (const auto& cell : mesh.GetEntities())
            {
                const std::size_t cell_index = cell.GetTopology().GetIndex();

                for (const auto& face : cell.GetTopology().GetSubEntities())
                {
                    const std::size_t face_index = face.GetTopology().GetLocalIndex();

                    // Default assignment: the containing cell itself.
                    lookup_face_neighboring_cell_mapping[cell_index][face_index] = cell_index;
                    lookup_face_neighboring_face_mapping[cell_index][face_index] = face_index;

                    // At most 2 incident cells: the containing cell and optionally a neighboring cell.
                    for (const auto& incident_cell : face.GetTopology().GetIncidentEntities())
                    {
                        const std::size_t incident_cell_index = incident_cell.GetTopology().GetIndex();

                        if (incident_cell_index != cell_index)
                        {
                            lookup_face_neighboring_cell_mapping[cell_index][face_index] = incident_cell_index;

                            for (const auto& neighboring_face : incident_cell.GetTopology().GetSubEntities())
                            {
                                if (neighboring_face == face)
                                {
                                    lookup_face_neighboring_face_mapping[cell_index][face_index] = neighboring_face.GetTopology().GetLocalIndex();

                                    break;
                                }
                            }

                            break;
                        }
                    }
                }
            }
#endif
        }

        protected:
        const MeshT& mesh;
        const std::size_t local_index;
        const std::size_t index;
        const std::size_t index_of_containing_cell;
    };
} // namespace HPM::entity

#endif