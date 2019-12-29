// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_ENTITIES_GEOMETRY_HPP
#define DSL_ENTITIES_GEOMETRY_HPP

#include <cmath>
#include <cstdint>
#include <tuple>

namespace HPM::mesh
{
    template <typename>
    class MeshAccessor;
}
 
namespace HPM::entity
{
    //!
    //! \brief Geometry base class.
    //!
    //! \tparam EntityT the type of the derived class implementation (CRTP)
    //! \tparam MeshT the mesh type
    //! \tparam CoordinateT the type of the nodes in the mesh
    //! \tparam EntityDimension the dimensionality of the entity (hierarchical definition)
    //!
    template <typename EntityT, typename MeshT, typename CoordinateT, std::size_t EntityDimension>
    class EntityGeometry
    {
        // Only entities with a dimension lower than that of the cells are allowed.
        static_assert(EntityDimension <= MeshT::CellDimension, "error: entity dimension must be lower than or equal to the cell dimension");

        protected:
        // Deduced types and constants.
        using ScalarT = typename CoordinateT::ValueT;
        using Geometry = typename EntityT::Geometry;
        using Topology = typename EntityT::Topology;
        static constexpr std::size_t WorldDimension = MeshT::WorldDimension;

        EntityGeometry() = default;

        //!
        //! \brief Constructor.
        //!
        //! This constructor takes a reference to the topology class associated with this entity.
        //!
        //! \param mesh a reference to the mesh
        //! \param topology a reference to the topology class associated with this entity
        //!
        EntityGeometry(const MeshT& mesh, const Topology& topology) : mesh(mesh), topology(topology) {}

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
        //! \brief Get the normal coordinates of this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the normal coordinates of this entity
        //!
        inline auto GetNormal() const
        {
#if defined(MESH_VALUE_LOOKUP)
            static_assert(WorldDimension == 3 && EntityDimension == 2, "error: implementation not available");

            // Case: face of a cell.
            if constexpr (Topology::IsFace && Topology::CreatedByCell)
            {
                const std::size_t cell_index = topology.GetIndexOfContainingCell();
                const std::size_t face_index = topology.GetLocalIndex();

                return std::get<2>(mesh.lookup_normals)[cell_index][face_index];
            }
#endif

            // CRTP: call the derived class' implementation.
            return static_cast<const Geometry&>(*this).GetNormalImplementation();
        }

        //!
        //! \brief Get the orientation of the normal vector.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the orientation of the normal
        //!
        inline auto GetNormalOrientation() const
        {
            // CRTP: call the derived class' implementation.
            return static_cast<const Geometry&>(*this).GetNormalOrientationImplementation();
        }

        //!
        //! \brief Get the unit-normal coordinates of this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the normal coordinates of this entity
        //!
        inline auto GetUnitNormal() const
        {
#if defined(MESH_VALUE_LOOKUP)
            static_assert(WorldDimension == 3 && EntityDimension == 2, "error: implementation not available");

            // Case: face of a cell.
            if constexpr (Topology::IsFace && Topology::CreatedByCell)
            {
                const std::size_t cell_index = topology.GetIndexOfContainingCell();
                const std::size_t face_index = topology.GetLocalIndex();

                return std::get<2>(mesh.lookup_unit_normals)[cell_index][face_index];
            }
#endif

            // CRTP: call the derived class' implementation.
            return static_cast<const Geometry&>(*this).GetUnitNormalImplementation();
        }

        //!
        //! \brief Get the length of the normal vector.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the length of the normal vector
        //!
        inline auto GetNormalLength() const
        {
#if defined(MESH_VALUE_LOOKUP)
            static_assert(WorldDimension == 3 && EntityDimension == 2, "error: implementation not available");

            // Case: face of a cell.
            if constexpr (Topology::IsFace && Topology::CreatedByCell)
            {
                const std::size_t cell_index = topology.GetIndexOfContainingCell();
                const std::size_t face_index = topology.GetLocalIndex();

                return std::get<2>(mesh.lookup_normal_lengths)[cell_index][face_index];
            }
#endif

            // CRTP: call the derived class' implementation.
            return static_cast<const Geometry&>(*this).GetNormalLengthImplementation();
        }

        //!
        //! \brief Get the center of this entity.
        //!
        //! \return the centroid (or geometric center) of the entity
        //!
        inline auto GetCenter() const
        {
            const auto& nodes = topology.GetNodes();
            CoordinateT center{};

            for (const auto& node : nodes)
            {
                center += node;
            }

            return center / nodes.size();
        }

        //!
        //! \brief Get the volume of this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the volume of this entity
        //!
        inline auto GetVolume() const { return static_cast<const Geometry&>(*this).GetVolumeImplementation(); }

        //!
        //! \brief Get the jacobian matrix for this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the jacobian matrix for this entity
        //!
        inline auto GetJacobian() const
        {
            return static_cast<const Geometry&>(*this).GetJacobianImplementation();
        }

        //!
        //! \brief Get the determinant of the jacobian matrix for this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the determinant of the jacobian matrix for this entity
        //!
        inline auto GetAbsJacobianDeterminant() const
        {
#if defined(MESH_VALUE_LOOKUP)
                            static_assert(EntityDimension == 3 && WorldDimension == 3, "error: implementation not available");

            return mesh.lookup_abs_jacobian_determinant[topology.GetIndex()];
#else
            return std::abs(GetJacobian().Determinant());
#endif
        }

        //!
        //! \brief Get the inverse of the jacobian matrix for this entity.
        //!
        //! CRTP: the derived class must provide an implementation.
        //!
        //! \return the inverse of the jacobian matrix for this entity
        //!
        inline auto GetInverseJacobian() const
        {
#if defined(MESH_VALUE_LOOKUP)
            static_assert(EntityDimension == 3 && WorldDimension == 3, "error: implementation not available");

            return mesh.lookup_inverse_jacobian[topology.GetIndex()];
#else
            return static_cast<const Geometry&>(*this).GetInverseJacobianImplementation();
#endif
        }

        //!
        //! \brief Setup data structures in the mesh that hold geometry information.
        //!
        //! \param mesh a reference to the mesh
        //!
        static auto SetupGeometry(MeshT& mesh)
        {
            // Setup data structures in the mesh: normals, for instance.
            Geometry::SetupGeometryImplementation(mesh);

#if defined(MESH_VALUE_LOOKUP)
            // Pre-calculate some geometry information.
            if constexpr (WorldDimension == 3 && EntityDimension == 3)
            {
                auto& lookup_abs_jacobian_determinant = mesh.lookup_abs_jacobian_determinant;
                auto& lookup_inverse_jacobian = mesh.lookup_inverse_jacobian;
                auto& lookup_normals = std::get<2>(mesh.lookup_normals);
                auto& lookup_unit_normals = std::get<2>(mesh.lookup_unit_normals);
                auto& lookup_normal_length = std::get<2>(mesh.lookup_normal_lengths);
                const std::size_t num_cells = mesh.GetNumEntities();

                if (lookup_abs_jacobian_determinant.empty())
                {
                    lookup_abs_jacobian_determinant.resize(num_cells);
                }

                if (lookup_inverse_jacobian.empty())
                {
                    lookup_inverse_jacobian.resize(num_cells);
                }

                if (lookup_normals.empty())
                {
                    lookup_normals.resize(num_cells);
                }

                if (lookup_unit_normals.empty())
                {
                    lookup_unit_normals.resize(num_cells);
                }

                if (lookup_normal_length.empty())
                {
                    lookup_normal_length.resize(num_cells);
                }

                auto& normals = mesh.normals[2];
                const auto& normal_orientations = std::get<2>(mesh.normal_orientations);

                for (const auto& cell : mesh.GetEntities())
                {
                    const std::size_t cell_index = cell.GetTopology().GetIndex();

                    lookup_abs_jacobian_determinant[cell_index] = std::abs(cell.GetGeometry().GetJacobianImplementation().Determinant());
                    lookup_inverse_jacobian[cell_index] = cell.GetGeometry().GetInverseJacobianImplementation();

                    for (const auto& face : cell.GetTopology().GetSubEntities())
                    {
                        const std::size_t face_index = face.GetTopology().GetLocalIndex();
                        const CoordinateT& face_normal = normals[face.GetTopology().GetIndex()];
                        const ScalarT orientation = normal_orientations[cell_index][face_index];

                        lookup_normals[cell_index][face_index] = face_normal * orientation;
                        lookup_unit_normals[cell_index][face_index] = Normalize(face_normal) * orientation;
                        lookup_normal_length[cell_index][face_index] = face_normal.Norm();
                    }
                }
            }
#endif
        }

        protected:
        const MeshT& mesh;
        const Topology& topology;
    };
} // namespace HPM::entity

#endif