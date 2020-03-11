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
#include <HighPerMeshes/dsl/data_access/LocalViewPlain.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsAccessDefinitions.hpp>

namespace C
{
#include <HighPerMeshes/dsl/data_access/LocalViewPlain.h>
}

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
        template <std::size_t Dimension, std::size_t NumDofs, typename EntityT, typename AccessDefinition>
        static auto GetLocalBuffer(const AccessDefinition& access, const EntityT& entity, [[maybe_unused]] const std::size_t offset, [[maybe_unused]] const std::size_t num_dofs)
        {
            // Perform calculations only if 'NumDofs > 0'.
            if constexpr (NumDofs > 0)
            {
                // The `LocalBuffer` type gets the compile-time `NumDofs` value for potential data transfer if a 'hard copy' of the dofs is requested.
                using LocalBuffer = typename AccessDefinition::template LocalBuffer<NumDofs>;

                // Get the considered entity.
                const auto& considered_entity = access.pattern(entity);

                // Request the (global) indices of all (sub-)entities of the specified `Dimension` (w.r.t. this entity).
                // Apply to each index the given transformation: calculate the offset to the base pointer associated with this buffer and set up a `LocalBuffer` pointing to this position.
                return TransformArray(considered_entity.GetTopology().template GetIndicesOfEntitiesWithDimension<Dimension>(), [&](const auto index) {
                    if constexpr (AccessDefinition::BufferT::DofT::IsConstexprArray)
                    {
                        return LocalBuffer(access.buffer, offset + NumDofs * index);
                    }
                    else
                    {
                        // NumDofs is either 0 or 1: use the num_dofs argument to provide the actual dof-value to the `LocalBuffer`.
                        assert(num_dofs > 0);

                        return LocalBuffer(access.buffer, offset + num_dofs * index, num_dofs);
                    }
                });
            }
            else
            {
                return InvalidLocalBuffer{};
            }
        }

        //!
        //! \brief Version of the `GetLocalBuffer` function for global dofs: no entity needed.
        //!
        template <std::size_t NumDofs, typename AccessDefinition>
        static auto GetLocalBuffer(const AccessDefinition& access, [[maybe_unused]] const std::size_t num_dofs)
        {
            using LocalBuffer = typename AccessDefinition::template LocalBuffer<NumDofs>;

            if constexpr (NumDofs > 0)
            {
                if constexpr (AccessDefinition::BufferT::DofT::IsConstexprArray)
                {
                    // Global dofs are located always at the beginning of the buffer.
                    return std::array<LocalBuffer, 1>{LocalBuffer(access.buffer, 0)};
                }
                else
                {
                    // NumDofs is either 0 or 1: use the num_dofs argument to provide the actual dof-value to the `LocalBuffer`.
                    assert(num_dofs > 0);

                    // Global dofs are located always at the beginning of the buffer.
                    return std::array<LocalBuffer, 1>{LocalBuffer(access.buffer, 0, num_dofs)};
                }
            }
            else
            {
                return InvalidLocalBuffer{};
            }
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
        template <typename AccessDefinition, typename EntityT, std::size_t... Dimension>
        static auto CreateImplementation(const AccessDefinition& access, const EntityT& entity, std::index_sequence<Dimension...>)
        {
            constexpr std::size_t CellDimension = EntityT::MeshT::CellDimension;

            // Get the considered entity type.
            using ConsideredElementT = std::decay_t<decltype(access.pattern(entity))>;

            // Deduce the dof mask from the access pattern.
            constexpr std::size_t DofMask[] = {(decltype(access.Dofs)::template At<Dimension>() && (Dimension <= ConsideredElementT::Dimension) ? 1 : 0)...};
            constexpr std::size_t GlobalDofMask = (decltype(access.Dofs)::template At<CellDimension + 1>() ? 1 : 0);

            // Use compile time Dofs?
            if constexpr (AccessDefinition::UseCompileTimeDofs)
            {
                // Extract the actual number of dofs from the `DofT` and the dof mask.
                using DofT = typename AccessDefinition::BufferT::DofT;
                constexpr std::size_t NumDofs[] = {DofT::template At<Dimension>() * DofMask[Dimension]...};
                constexpr std::size_t NumGlobalDofs = DofT::template At<CellDimension + 1>() * GlobalDofMask;

                // Get all `LocalBuffer`s or `InvalidLocalBuffer`s for this entity.
                return std::tuple{GetLocalBuffer<Dimension, NumDofs[Dimension]>(access, entity, GetOffset<Dimension, DofT>(entity.GetMesh()), NumDofs[Dimension])...,
                                    GetLocalBuffer<NumGlobalDofs>(access, NumGlobalDofs),
                                    std::integral_constant<std::size_t, CellDimension>{}};
            }
            else
            {
                // Get the dofs from the access list element.
                const auto& dofs = access.buffer->GetDofs();
                const std::size_t num_dofs[] = {dofs.template At<Dimension>() * DofMask[Dimension]...};
                const std::size_t num_global_dofs = dofs.template At<CellDimension + 1>() * GlobalDofMask;

                // Get all `LocalBuffer`s or `InvalidLocalBuffer`s for this entity.
                // If DofMask[] > 0 holds, num_dofs[] is used to access the actual dof-values.
                return std::tuple{GetLocalBuffer<Dimension, DofMask[Dimension]>(access, entity, GetOffset<Dimension>(entity.GetMesh(), dofs), num_dofs[Dimension])...,
                                    GetLocalBuffer<GlobalDofMask>(access, num_global_dofs),
                                    std::integral_constant<std::size_t, CellDimension>{}};
            }
        }

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
            -> std::tuple<decltype(CreateImplementation(std::get<TupleIndex>(access_definitions), entity, std::make_index_sequence<EntityT::MeshT::CellDimension + 1>{}))...>
        {
            // Create the local view per access list element: for each access list element iterate over all dimensions up to the cell dimension.
            return {CreateImplementation(std::get<TupleIndex>(access_definitions), entity, std::make_index_sequence<EntityT::MeshT::CellDimension + 1>{})...};
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

        //!
        //! \brief Create a local view to the dofs of some entity according to some access list.
        //!
        //! This function translates the `LocalBuffer` version of the local view into a plain data type which instead of the `LocalBuffers` stores offsets to some base pointer.
        //!
        //! \tparam AccessDefinitions the `AccessDefinitionDefinition` type for a collection of fields
        //! \tparam EntityT the type of the considered entity
        //! \param access_definitions the `AccessDefinitionDefinition` for a collection of fields
        //! \param entity the considered entity
        //! \return a tuple of tuples of `LocalBuffer` or `InvalidLocalBuffer` instances
        //!
        template <typename AccessDefinitions, typename EntityT>
        static auto CreatePlain(const AccessDefinitions& access_definitions, const EntityT& entity)
        {
            using TupleT = decltype(Create(access_definitions, entity, std::make_index_sequence<std::tuple_size_v<AccessDefinitions>>{}));

            return LocalViewPlain::Create<TupleT>(access_definitions, entity.GetMesh(), entity);
        }
    };
} // namespace HPM::internal

#endif