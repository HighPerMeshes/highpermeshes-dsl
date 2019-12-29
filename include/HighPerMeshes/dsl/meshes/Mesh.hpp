// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_MESHES_MESH_HPP
#define DSL_MESHES_MESH_HPP

#include <array>
#include <cstdint>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#define MESH_VALUE_LOOKUP

#include <HighPerMeshes/common/IndexSequence.hpp>
#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/entities/Geometry.hpp>
#include <HighPerMeshes/dsl/entities/Topology.hpp>
#include <HighPerMeshes/dsl/meshes/Range.hpp>

namespace HPM::mesh
{
    //!
    //! \brief Mesh data type.
    //!
    //! Mesh entities are defined hierarchically and in categories, e.g., Simplex:
    //!      3-Simplex = tetrahedron,
    //!      2-Simplex = triangle,
    //!      1-Simplex = edge,
    //!      0-Simplex = node.
    //! All of them are different types.
    //! The mesh provides member functions to iterate over these types.
    //! Entities themselves provide topology (neighbor-/incident-/../sub-entities) and geometry (normal vectors and directsions) information.
    //! For that, entities must implement both the `EntityGeometry` and `EntityTopology` base class.
    //!
    //! \tparam CoordinateT the coordinate type used for the node (vertex) representation
    //! \tparam EntityTypeName the class type of the mesh entities
    //! \tparam CellDimension the dimensionality of the mesh entity type 'cell' (can be lower than the coordinate dimension)
    //!
    template <typename CoordinateT, template <typename, typename, std::size_t, typename> typename EntityTypeName, std::size_t CellDimension_ = CoordinateT::Dimension>
    class Mesh
    {
        static_assert(::HPM::dataType::internal::ProvidesMetaData<CoordinateT>::value, "error: ScalarT and Dimension meta data is required");

        protected:
        // Indicator for mesh entity type 'cell' with invalid index.
        using NullT = void;

        public:
        // Template arguments.
        static constexpr std::size_t CellDimension = CellDimension_;

        // Deduced types and constants.
        using ScalarT = typename CoordinateT::ValueT;
        using CellT = EntityTypeName<Mesh, CoordinateT, CellDimension, NullT>;
        template <std::size_t Dimension>
        using EntityT = EntityTypeName<Mesh, CoordinateT, Dimension, NullT>;

        static constexpr std::size_t WorldDimension = CoordinateT::Dimension;
        static_assert(CellDimension > 0, "error: this is a node set");
        static_assert(CellDimension <= WorldDimension, "error: that does not make sense! entity dimension > world_dimension?");

        static constexpr std::size_t InvalidIndex = std::numeric_limits<std::size_t>::max();

        protected:
        // Deduced types and constants.
        template <std::size_t Dimension>
        using EntityRange = ::HPM::iterator::EntityRange<EntityT<Dimension>, Mesh>;
        // The last template paramter is set to 'true' which results in the local index of an entity equals its (global) index.
        // This is needed because entities of any dimension and with no embedding can be created by the mesh.
        // The equality of the local index and the index of an entity is enforced internally only for cells.
        template <std::size_t Dimension>
        using IndexedEntityRange = ::HPM::iterator::IndexedEntityRange<EntityT<Dimension>, Mesh, true>;

        //!
        //! \brief A data type to request the number of nodes of an entity.
        //!
        struct NumNodesOfEntity
        {
            //!
            //! \brief Get the number of nodes of an entity.
            //!
            //! \tparam Dimension the entity dimension
            //! \return the number of nodes of the entity with the specified dimension
            //!
            template <std::size_t Dimension>
            static constexpr auto WithDimension()
            {
                return NumNodesOfEntity{}(std::integral_constant<std::size_t, Dimension>{});
            }

            //!
            //! \brief Get the number of nodes of an entity.
            //!
            //! \tparam Dimension the value of the integral constant
            //! \param unnamed an integral constant
            //! \return the value of the integral constant
            //!
            template <std::size_t Dimension>
            constexpr auto operator()(const std::integral_constant<std::size_t, Dimension>&)
            {
                // Nodes are entities with dimension 0.
                return EntityT<Dimension>::Topology::template GetNumEntities<0>();
            }
        };

        //!
        //! \brief A data type to request the number of sub-entities within a cell.
        //!
        struct NumSubEntitiesOfCell
        {
            //!
            //! \brief Get the number of sub-entities within a cell.
            //!
            //! \tparam Dimension the sub-entity dimension
            //! \return the number of sub-entities within a cell.
            //!
            template <std::size_t Dimension>
            static constexpr auto WithDimension()
            {
                return NumSubEntitiesOfCell{}(std::integral_constant<std::size_t, Dimension>{});
            }

            //!
            //! \brief Get the number of sub-entities within a cell.
            //!
            //! \tparam Dimension the value of the integral constant
            //! \param unnamed an integral constant
            //! \return the value of the integral constant
            //!
            template <std::size_t Dimension>
            constexpr auto operator()(const std::integral_constant<std::size_t, Dimension>&)
            {
                return CellT::Topology::template GetNumEntities<Dimension>();
            }
        };

        static constexpr std::size_t NumNodesPerCell = NumNodesOfEntity::template WithDimension<CellDimension>();
        static constexpr std::size_t NumFacesPerCell = NumSubEntitiesOfCell::template WithDimension<CellDimension - 1>();
        static constexpr std::size_t NumNodesPerFace = NumNodesOfEntity::template WithDimension<CellDimension - 1>();

        // Friend declarations: needed for member access.
        template <template <typename, typename, std::size_t, typename> typename, typename, typename, std::size_t, typename>
        friend class ::HPM::entity::EntityTopology;
        template <typename, typename, typename, std::size_t>
        friend class ::HPM::entity::EntityGeometry;

        public:
        //!
        //! \brief Constructor.
        //!
        //! Create a mesh form a node set and a cell to node mapping.
        //!
        //! \param nodes a set of nodes
        //! \param cell_node_index_list the mapping of the cells to the nodes
        //!
        Mesh(const std::vector<CoordinateT>& nodes, const std::vector<std::array<std::size_t, NumNodesPerCell>>& cell_node_index_list) : nodes(nodes)
        {
            std::get<CellDimension>(entity_node_index_list) = cell_node_index_list;

            // Set up internal data members according to the entity type: the cell type must be used!
            CellT::Topology::SetupTopology(*this);
            CellT::Geometry::SetupGeometry(*this);
        }

        //!
        //! \brief Create a mesh from a mesh file.
        //!
        //! This function extracts the nodes and the cell to node mappings from a mesh file,
        //! and creates the mesh using this data.
        //!
        //! \tparam ReaderT the class type of the mesh file reader
        //! \param filename the name of the mesh file
        //! \return a mesh object
        //!
        template <template <typename, typename> class ReaderT>
        static auto CreateFromFile(const std::string& filename) -> Mesh
        {
            using NodeIndexT = std::array<std::size_t, NumNodesPerCell>;
            using MeshReader = ReaderT<CoordinateT, NodeIndexT>;
            auto&& fields = MeshReader().ReadNodesAndElements(filename);

            return {std::get<0>(fields), std::get<1>(fields)};
        }

        //!
        //! \brief Get the number of entities with a given dimension.
        //!
        //! \tparam Dimension the entity dimension
        //! \return the number of entities with the specified dimension
        //!
        template <std::size_t Dimension = CellDimension>
        inline auto GetNumEntities() const
        {
            // Each entity has a unique set of node indices.
            // The number of node index sets determines the number of entities.
            return std::get<Dimension>(entity_node_index_list).size();
        }

        //!
        //! \brief Get an iterator over all entities with a given dimension in the range [begin, end).
        //!
        //! \tparam Dimension the entity dimension
        //! \param begin the begin of the range
        //! \param end the end of the range
        //! \return an iterator over all entities with a given dimension
        //!
        template <std::size_t Dimension = CellDimension>
        inline auto GetEntities(const std::size_t begin_default = InvalidIndex, const std::size_t end_default = InvalidIndex) const -> EntityRange<Dimension>
        {
            const std::size_t begin = (begin_default == InvalidIndex ? 0 : (end_default == InvalidIndex ? 0 : begin_default));
            const std::size_t end = (begin_default == InvalidIndex ? GetNumEntities<Dimension>() : (end_default == InvalidIndex ? begin_default : end_default));
            
            assert(begin <= end);

            return {*this, begin, end};
        }

        //!
        //! \brief Get an iterator over entities with a given dimension and (global) indices.
        //!
        //! \tparam Dimension the entity dimension
        //! \param indices the (global) indices of the entities
        //! \return an iterator over all entities with a given dimension
        //!
        template <std::size_t Dimension = CellDimension>
        inline auto GetEntities(const std::vector<std::size_t>& indices) const -> IndexedEntityRange<Dimension>
        {
            return {*this, indices};
        }

        //!
        //! \brief Get all entities of a given dimension.
        //!
        //! \tparam RangeT the range type
        //! \param range the range of the (global) entity indices
        //! \param partition the index of the partition: meaningful for the PartitionedMesh only
        //! \return an iterator over all entities according to the entity `range` assigned to this process
        //!
        template <typename RangeT>
        inline auto GetEntities(const RangeT& range) const
        {
            const auto& mesh = range.GetMesh();

            if (&mesh != this)
            {
                throw std::runtime_error("error: the range has not been created by this mesh");
            }

            return GetEntities<RangeT::EntityDimension>(range.GetIndices());
        }

        //!
        //! \brief Get an entity range.
        //!
        //! This function returns a `Range` type with entity indices according to the evaluation of a callable.
        //! The callable is evaluation for all entities of the specified dimension and the provided (global) indices.
        //! If the callable evaluates to `true`, the (global) entity index is added to the index vector.
        //! Otherwise, it is left out.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \tparam FuncT the type of the callable
        //! \param func a callable
        //! \param indices the entity indices to be considered
        //! \return a `Range` type according to the entity dimension and the evaluation of the callable
        //!
        template <std::size_t Dimension, typename FuncT>
        inline auto GetEntityRange(FuncT func, const std::vector<std::size_t>& indices) const
        {
            static_assert(Dimension <= CellDimension, "error: requested dimension is larger than the cell dimension");

            std::set<std::size_t> entity_indices;

            for (const auto& entity : GetEntities<Dimension>(indices))
            {
                static_assert(std::is_invocable_v<FuncT, decltype(entity)>, "error: callable argument type mismatch");
                static_assert(std::is_same_v<decltype(func(entity)), bool>, "error: callable does not return a bool");

                // Evaluate the callable.
                if (func(entity))
                {
                    // Add the (global) index of this entity to the index set.
                    entity_indices.insert(entity.GetTopology().GetIndex());
                }
            }

            return MakeRange<Dimension>(*this, std::move(entity_indices));
        }

        //!
        //! \brief Get an entity range.
        //!
        //! Wrapper: all entity indices are added to the index set.
        //! 
        //! \tparam Dimension the dimension of the entity
        //! \param indices the entity indices to be considered
        //! \return a `Range` type according to the entity dimension
        //! 
        template <std::size_t Dimension>
        inline auto GetEntityRange(const std::vector<std::size_t>& indices) const
        {
            return GetEntityRange<Dimension>([](const auto&) { return true; }, indices);
        }

        //!
        //! \brief Get an entity range.
        //!
        //! Wrapper: the index range is between `begin` and `end`.
        //!
        //! \tparam Dimension the dimension of the entity
        //! \tparam FuncT the type of the callable
        //! \param func a callable
        //! \param begin the begin of the range
        //! \param end the end of the range
        //! \return a `Range` type according to the entity dimension and the evaluation of the callable
        //!
        template <std::size_t Dimension, typename FuncT>
        inline auto GetEntityRange(FuncT func, const std::size_t begin_default = InvalidIndex, const std::size_t end_default = InvalidIndex) const
        {
            const std::size_t begin = (begin_default == InvalidIndex ? 0 : (end_default == InvalidIndex ? 0 : begin_default));
            const std::size_t end = (begin_default == InvalidIndex ? GetNumEntities<Dimension>() : (end_default == InvalidIndex ? begin_default : end_default));

            assert(begin <= end);

            const std::size_t num_entities = end - begin;
            std::vector<std::size_t> indices(num_entities);

            for (std::size_t i = 0; i < num_entities; ++i)
            {
                indices[i] = begin + i;
            }

            return GetEntityRange<Dimension>(func, indices);
        }

        //!
        //! \brief Get an entity range.
        //!
        //! Wrapper: all entity indices between `begin` and `end` are added to the index set.
        //! 
        //! \tparam Dimension the dimension of the entity
        //! \param begin the begin of the range
        //! \param end the end of the range
        //! \return a `Range` type according to the entity dimension
        //! 
        template <std::size_t Dimension>
        inline auto GetEntityRange(const std::size_t begin = InvalidIndex, const std::size_t end = InvalidIndex) const
        {
            return GetEntityRange<Dimension>([](const auto&) { return true; }, begin, end);
        }

        protected:
        //!
        //! \brief Accessor data structure.
        //!
        //! This data structure is an implicit friend of the mesh and provides access to the mesh's private/protected members.
        //! It is used by the implementing entity classes to access the mesh's members, because a friend
        //! declaration of these classes inside the mesh would shadow the mesh's `EntityTypeName` template parameter
        //! (Clang says, it is a redeclaration of the template parameter; GNU does not complain).
        //!
        //! Note: only the `EntityTopology` and `EntityGeometry` classes can instantiate this type.
        //! The use of this data structure for member access is more a work around then.
        //!
        //! \tparam MeshT the mesh type (can be `const` or non-`const`)
        //!
        template <typename MeshT>
        class MemberAccessor
        {
            // Friend declarations: needed for instantiation.
            template <template <typename, typename, std::size_t, typename> typename, typename, typename, std::size_t, typename>
            friend class ::HPM::entity::EntityTopology;
            template <typename, typename, typename, std::size_t>
            friend class ::HPM::entity::EntityGeometry;

            //!
            //! \brief Constructor.
            //!
            //! \param mesh a (const) reference to the mesh
            //!
            MemberAccessor(MeshT& mesh) : mesh(mesh) {}

            public:
            //!
            //! \brief Get access to the `nodes` member.
            //!
            //! \return a (const) reference to the `nodes` member
            //!
            inline auto& Nodes() const { return mesh.nodes; }

            //!
            //! \brief Get access to the `entity_node_index_list` member.
            //!
            //! \return a (const) reference to the `entity_node_index_list` member
            //!
            inline auto& EntityNodeIndexList() const { return mesh.entity_node_index_list; }

            //!
            //! \brief Get access to the `entity_index_list` member.
            //!
            //! \return a (const) reference to the `entity_index_list` member
            //!
            inline auto& EntityIndexList() const { return mesh.entity_index_list; }

            //!
            //! \brief Get access to the `entity_incidence_list` member.
            //!
            //! \return a (const) reference to the `entity_incidence_list` member
            //!
            inline auto& EntityIncidenceList() const { return mesh.entity_incidence_list; }

            //!
            //! \brief Get access to the `entity_boundary_list` member.
            //!
            //! \return a (const) reference to the `entity_boundary_list` member
            //!
            inline auto& EntityBoundaryList() const { return mesh.entity_boundary_list; }

            //!
            //! \brief Get access to the `entity_neighbor_list` member.
            //!
            //! \return a (const) reference to the `entity_neighbor_list` member
            //!
            inline auto& EntityNeighborList() const { return mesh.entity_neighbor_list; }

            //!
            //! \brief Get access to the `normals` member.
            //!
            //! \return a (const) reference to the `normals` member
            //!
            inline auto& Normals() const { return mesh.normals; }

            //!
            //! \brief Get access to the `normal_orientations` member.
            //!
            //! \return a (const) reference to the `normal_orientations` member
            //!
            inline auto& NormalOrientations() const { return mesh.normal_orientations; }

            private:
            MeshT& mesh;
        };

        //!
        //! \brief A list type to hold multiple `std::arrays` with different extent.
        //!
        //! \tparam T the element type of the `std::arrays`
        //! \tparam IndexT an index type
        //!
        template <typename T, typename IndexT>
        struct ListT;

        //!
        //! \brief A list type to hold multiple `std::arrays` with different extent.
        //!
        //! \tparam T the element type of the `std::arrays`
        //! \tparam I a parameter pack: a sequence of integer values
        //!
        template <typename T, std::size_t... I>
        struct ListT<T, std::integer_sequence<std::size_t, I...>>
        {
            using type = std::tuple<std::vector<std::array<T, I>>...>;
        };

        template <typename T>
        using EntityNodeIndexSetT = typename ListT<T, ::HPM::dataType::IndexSequence<CellDimension + 1, NumNodesOfEntity>>::type;
        template <typename T>
        using EntityIndexListT = typename ListT<T, ::HPM::dataType::IndexSequence<CellDimension + 1, NumSubEntitiesOfCell>>::type;

        std::vector<CoordinateT> nodes;                                                               //       vector<CoodinateT>
        EntityNodeIndexSetT<std::size_t> entity_node_index_list;                                      // tuple<vector<array<size_t, NumNodesOfNodes<0>>,..,array<size_t, NumNodesOfNodes<CellDimension>>>>
        EntityIndexListT<std::size_t> entity_index_list;                                              // tuple<vector<array<size_t, NumSubEntitiesOfCell<0>>,..,array<size_t, 1>>>
        std::array<std::vector<std::vector<std::size_t>>, CellDimension + 1> entity_incidence_list;   // array<vector<vector<size_t>>, CellDimension+1>
        std::array<std::vector<std::size_t>, CellDimension + 1> entity_boundary_list;                 // array<vector<size_t>, CellDimension+1>
        std::array<std::vector<std::vector<std::size_t>>, CellDimension + 1> entity_neighbor_list;    // array<vector<vector<size_t>>, CellDimension+1>
        std::array<std::vector<CoordinateT>, CellDimension + 1> normals;                              // array<vector<CoordinateT>, CellDimension+1>
        EntityIndexListT<ScalarT> normal_orientations;                                                // tuple<vector<array<ScalarT, NumSubEntitiesOfCell<0>>,..,array<ScalarT, 1>>>

#if defined(MESH_VALUE_LOOKUP)
        template <typename T, std::size_t M, std::size_t N>
        using MatrixT = ::HPM::dataType::Matrix<ScalarT, M, N>;

        std::vector<ScalarT> lookup_abs_jacobian_determinant;
        std::vector<MatrixT<ScalarT, WorldDimension, WorldDimension>> lookup_inverse_jacobian;
        EntityIndexListT<CoordinateT> lookup_normals;
        EntityIndexListT<CoordinateT> lookup_unit_normals;
        EntityIndexListT<ScalarT> lookup_normal_lengths;
        std::vector<std::array<std::size_t, NumFacesPerCell>> lookup_face_neighboring_cell_mapping;
        std::vector<std::array<std::size_t, NumFacesPerCell>> lookup_face_neighboring_face_mapping;
#endif
    };
} // namespace HPM::mesh

#endif