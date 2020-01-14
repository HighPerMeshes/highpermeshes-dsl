#ifndef DSL_MESHES_RANGE_HPP
#define DSL_MESHES_RANGE_HPP

#include <set>
#include <vector>

namespace HPM::mesh
{
    template <std::size_t EntityDimension, typename MeshT, typename ContainerT>
    auto MakeRange(const MeshT& mesh, const ContainerT&& indices);

    //!
    //! \brief A range data type that holds a mesh reference and a nested index vector for the entity creation.
    //!
    //! The index vector is always for a level-2 (L2) partitioning, that is, there is an index vector for each L2 partition.
    //! The Mesh type does not know about L2 partitions: it thus has only 1 index vector.
    //!
    //! Usage: request an object of this type from the PartitionedMesh via `GetEntityRange(..)` and
    //! input the index vector into the Mesh for the actual iteration over all entities:
    //! ```
    //!    const auto& range_all = mesh.template GetEntityRange<2>();
    //!    for (std::size_t i_L2 = 0; i_L2 < num_L2_partitions; ++i_L2)
    //!        for (const auto& entity : mesh.RangeToEntity<range::EntityDimension>(range_all.GetIndices(i_L2)))
    //!        {
    //!            entity.GetTopology()..;
    //!        }
    //!    const auto& range_sub = mesh.template GetEntityRange<2>([] (const auto& entity) { return SomePredicate(entity); });
    //!    for (const auto& entity : mesh.RangeToEntity<range::EntityDimension>(range_sub.GetIndices()))
    //!    {
    //!        entity.GetTopology()..;
    //!    }
    //! ```
    //!
    //! \tparam EntityDimension the dimension of the entities to be created
    //! \tparam MeshT the mesh type
    //!
    template <std::size_t EntityDimension_, typename MeshT>
    class Range
    {
        public:
            static constexpr auto EntityDimension = EntityDimension_;
        private:

        // Only the `PartitionedMesh` and the `Mesh` can instantiate this type: the index vector is always valid!
        template <typename, template <typename, typename, std::size_t, typename> typename, std::size_t>
        friend class PartitionedMesh;
        template <typename, template <typename, typename, std::size_t, typename> typename, std::size_t>
        friend class Mesh;
        friend auto MakeRange<EntityDimension, MeshT, std::set<std::size_t>>(const MeshT&, const std::set<std::size_t>&&);
        friend auto MakeRange<EntityDimension, MeshT, std::vector<std::size_t>>(const MeshT&, const std::vector<std::size_t>&&);

        //!
        //! \brief Constructor.
        //!
        //! Only the `PartitionedMesh` can instantiate this type: the index vector is always valid!
        //!
        //! \param mesh a reference to the mesh
        //! \param indices an index vector to be used for the entity creation
        //!
        Range(const MeshT& mesh, const std::vector<std::vector<std::size_t>>&& indices) : mesh(mesh), indices(std::move(indices)) {}

        public:
        //!
        //! \brief Get a mesh reference.
        //!
        //! \return a mesh reference
        //!
        inline const auto& GetMesh() const { return mesh; }

        //!
        //! \brief Get the index vector of a specific partition.
        //!
        //! \param partition the index of the partition
        //! \return a reference to the index vector of the specified partition
        //!
        inline const auto& GetIndices(const std::size_t partition = 0) const { return indices.at(partition); }

        //!
        //! \brief Get entities within a partition.
        //!
        //! \param partition the index of the partition
        //! \return an iterable set of entities specified by the given `mesh`, `indices` and `EntityDimension`  
        //!
        inline auto GetEntities(const std::size_t partition = 0) const 
        {
            return mesh.template GetEntities<EntityDimension>(GetIndices(partition));
        }

        private:
        const MeshT& mesh;
        const std::vector<std::vector<std::size_t>> indices;
    };

    template <std::size_t EntityDimension, typename MeshT, typename ContainerT>
    inline auto MakeRange(const MeshT& mesh, const ContainerT&& indices)
    {
        return Range<EntityDimension, MeshT>(mesh, std::move(std::vector<std::vector<std::size_t>>{{indices.begin(), indices.end()}}));
    }
} // namespace HPM::mesh

#endif