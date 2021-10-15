// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_LOCALVIEW_HPP
#define DSL_DATAACCESS_LOCALVIEW_HPP

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>

#include <HighPerMeshes/auxiliary/ArrayOperations.hpp>
#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>
#include <HighPerMeshes/dsl/buffers/LocalBuffer.hpp>
#include <HighPerMeshes/dsl/data_access/Dof.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsAccessDefinitions.hpp>

namespace HPM::internal
{
    using ::HPM::auxiliary::TransformArray;
    using ::HPM::dof::GetOffset;

    //!
    //! \brief The local view is a collection of `LocalBuffer`s each of which pointing to some arbitrary position in main memory associated with then entity dofs.
    //!
    class LocalView
    {
        //!
        //! \brief Construct the `LocalBuffer`s.
        //!
        //! For each dimenson with `NumDofs > 0` request the indices of the entities of dimension `Dimension` and calculate the relative positions (offsets) to global base buffer `data.buffer`.
        //! Use these positions to set up the local buffers.
        //!
        //! \tparam Dimension the dimension of the considered entity (w.r.t. the provided `entity`)
        //! \tparam NumDofs the number of dofs in this dimension: defined together with this buffer, and needed for the offset calculation
        //! \tparam EntityT the type of the considered entity
        //! \tparam AccessDefinition the `AccessDefinitionDefinition` type for a particular field (element)
        //! \param access the `AccessDefinitionDefinition` for a particular field (element)
        //! \param entity the considered entity
        //! \param offset an offset value pointing into the partition of dofs in this dimension
        //! \param num_dofs the number of dofs in the dimension
        //! \return a `LocalBuffer` instance if the number of dofs requested is larger than zero, otherwise an `InvalidLocalBuffer` type
        //!
        template <std::size_t Dimension, typename EntityT, typename AccessDefinition>
        static auto GetLocalBuffer(const AccessDefinition& access, const EntityT& considered_entity, const std::size_t num_dofs, size_t offset)
        {
            // Request the (global) indices of all (sub-)entities of the specified `Dimension` (w.r.t. this entity).
            // Apply to each index the given transformation: calculate the offset to the base pointer associated with this buffer and set up a `LocalBuffer` pointing to this position.
            return TransformArray(considered_entity.GetTopology().template GetIndicesOfEntitiesWithDimension<Dimension>(), [&](const auto index) {
                return LocalBuffer { access.buffer, access.Mode, offset + num_dofs * index };
            });

        }

        //!
        //! \brief Version of the `GetLocalBuffer` function for global dofs: no entity needed.
        //!
        template <typename AccessDefinition>
        static auto GetLocalBuffer(const AccessDefinition& access, std::size_t num_dofs)
        {
            return LocalBuffer { access.buffer, access.Mode, 0 };
        }

        //!
        //! \brief Implementation of the local view creation (processing of each element of the access list indivually).
        //!
        //! For each element of the access list (a tuple type) and dimenions up to the cell dimension, create arrays with as many local buffers as there are (sub-)entities in that dimension.
        //!
        //! \tparam AccessDefinition the `AccessDefinitionDefinition` type for a particular field (element)
        //! \tparam EntityT the type of the considered entity
        //! \tparam Dimension a variadic list of dimensions to be consdered: 0..CellDimension
        //! \param access the `AccessDefinitionDefinition` for a particular field (element)
        //! \param entity the considered entity
        //! \param unnamed used for template parameter deduction
        //! \return a tuple of `LocalBuffer` or `InvalidLocalBuffer` instances
        //!
        template <typename AccessDefinition, typename EntityT>
        static auto CreateImplementation(const AccessDefinition& access, const EntityT& entity)
        {
            constexpr size_t CellDimension = EntityT::MeshT::CellDimension;

            constexpr size_t RequestedDofDimension = AccessDefinition::RequestedDimension;

            const auto& dofs = access.buffer->GetDofs();
            
            // Get the considered entity.
            const auto& considered_entity = access.pattern(entity);
            constexpr size_t ConsideredDimension = std::decay_t<decltype(considered_entity)>::Dimension;

            assert(dofs.Get()[RequestedDofDimension] != 0);

            if constexpr(RequestedDofDimension == CellDimension + 1) {
                return GetLocalBuffer(access, dofs.At(RequestedDofDimension));
            }
            else {
                if constexpr(ConsideredDimension == RequestedDofDimension) {
                    // In this case, there can only be one entity, therefore we can directly return the first entry to make life easier on the user side.
                    return GetLocalBuffer<RequestedDofDimension>(access, considered_entity, dofs.At(RequestedDofDimension), GetOffset<RequestedDofDimension>(entity.GetMesh(), dofs))[0];
                }
                else {
                    return GetLocalBuffer<RequestedDofDimension>(access, considered_entity, dofs.At(RequestedDofDimension), GetOffset<RequestedDofDimension>(entity.GetMesh(), dofs));
                }
            }
        }

        template<size_t... DofDimension> struct DimensionalityList {};


        //!
        //! \brief Implementation of the local view creation (processing of the entire access list).
        //!
        //! This function just calls the creation function for each element of the access list and dimensions up to the cell dimension.
        //!
        //! \tparam AccessDefinitions the `AccessDefinitionDefinition` type for a collection of fields
        //! \tparam EntityT the type of the considered entity
        //! \tparam TupleIndex a variadic list of tuple indices: 0..tuple-size
        //! \param access_definitions the `AccessDefinitionDefinition` for a collection of fields
        //! \param entity the considered entity
        //! \param unnamed used for template parameter deduction
        //! \return a tuple of tuples of `LocalBuffer` or `InvalidLocalBuffer` instances
        //!
        template <typename AccessDefinitions, typename EntityT, std::size_t... TupleIndex>
        static auto Create(const AccessDefinitions& access_definitions, const EntityT& entity, std::index_sequence<TupleIndex...>)
        {
            // Create the local view per access list element: for each access list element iterate over all dimensions up to the cell dimension.
            return std::tuple{CreateImplementation(std::get<TupleIndex>(access_definitions), entity)..., DimensionalityList<std::decay_t<decltype(std::get<TupleIndex>(access_definitions))>::RequestedDimension...> {} };
        }

        public:
        //!
        //! \brief Create a local view to the dofs of some entity according to some access list.
        //!
        //! The access list is a tuple of `AccessDefinitionDefinition`s for a collection of fields.
        //! The dof access (into these fields) always happens w.r.t. some entity through `LocalBuffer`s that point to the dofs of the (sub-)entities that are requested.
        //!
        //! \tparam AccessDefinitions the `AccessDefinitionDefinition` type for a collection of fields
        //! \tparam EntityT the type of the considered entity
        //! \param access_definitions the `AccessDefinitionDefinition` for a collection of fields
        //! \param entity the considered entity
        //! \return a tuple of tuples of `LocalBuffer` or `InvalidLocalBuffer` instances
        //!
        template <typename AccessDefinitions, typename EntityT>
        static auto Create(const AccessDefinitions& access_definitions, const EntityT& entity)
        {
            static_assert(IsAccessDefinitions<AccessDefinitions>);

            // Iterate over all ('C_TupleSize') tuple entities: use an index_sequence for the entity access.
            return Create(access_definitions, entity, std::make_index_sequence<std::tuple_size_v<AccessDefinitions>>{});
        }

        template <typename AccessDefinitions, typename EntityIterator, std::size_t ...I>
        static auto CreateMultiple(const AccessDefinitions& access_definitions, const EntityIterator& it, std::index_sequence<I...>)
            -> std::array<decltype(Create(access_definitions, it[0])), sizeof...(I)>
        {
            return {Create(access_definitions, it[I])...};
        }
    };
} // namespace HPM::internal

#endif
