// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_LOOPTYPES_LOOPIMPLEMENTATIONS_DEFAULTLOOPIMPLEMENTATIONS_HPP
#define DSL_LOOPTYPES_LOOPIMPLEMENTATIONS_DEFAULTLOOPIMPLEMENTATIONS_HPP

#include <cstdint>
#include <utility>

#include <HighPerMeshes/dsl/data_access/LocalView.hpp>
#include <HighPerMeshes/dsl/loop_types/ExecutionPolicy.hpp>

//!
//! \name
//! LoopImplementations
//!
//! LoopImplementations provide an implementation for the two loop types `ForEachEntity` and `ForEachIncidence` that are used in HPM.
//! They are functors that are invoked by the run time system at different points in the project but may also be interchanged,
//! for example to target different architectures.
//!
//! \{
namespace HPM::internal
{
    using ::HPM::ExecutionPolicy;

    //! \brief
    //! The default implementation for the `ForEachEntity` loop
    //!
    //! \tparam
    //! Dimension_ specifies the dimension of requested entities to iterate over
    //!
    template <std::size_t Dimension_, ExecutionPolicy Policy_>
    struct ForEachEntity
    {
        template <typename IteratorT, std::size_t...I>
        inline auto GetEntities(IteratorT& it, const std::size_t max_index, std::index_sequence<I...>) const -> std::array<typename IteratorT::EntityT, sizeof...(I)>
        {
            return {it[std::min(I, max_index)]...};
        }

        //! Is used to infer the dimension of entities in operator() by the run time system.
        static constexpr std::size_t Dimension = Dimension_;

        static constexpr ExecutionPolicy Policy = Policy_;

        //! \brief Iterates over all given `entities` and invokes `loop_body` for each of them.
        //!
        //! For example, given a vector of edges `e`, operator()(e, [](auto edge) {}) iterates over all edges in `e` and invokes `loop_body` for each of them.
        //!
        //! \attention
        //! `loop_body` must be invocable with the entities in `entities`
        template <typename EntityRange, typename LoopBody>
        auto operator()(EntityRange&& entities, LoopBody loop_body) const
        {
            for (const auto& entity : std::forward<EntityRange>(entities))
            {
                loop_body(entity);
            }
        }

        //! \brief Iterates over all given `entities` and invokes `loop_body` for each of them.. It further constructs the appropriate local vectors for the defined access definitions.
        //!
        //! For example, given a vector of edges `e` and the SubDimension for nodes, operator()(e, [](auto node, auto local_view) {}) iterates over all child-nodes of each edge in `e` and invokes `loop_body` for each
        //! of the nodes.
        //!
        //! \attention
        //! `loop_body` must be invocable with the requested sub-entities and local_vectors
        //!
        //! \note
        //! Since `loop_body` is invoked for each child-entity of each parent-entity in `entities`, the same sub-entity is called more than once with different parent-entities.
        //! For example, a node belongs to multiple edges and is therefore visited multiple times.
        //!
        //! \see
        //! AccessDefinition.hpp
        template <typename EntityRange, typename AccessDefinitions, typename LoopBody>
        auto operator()(EntityRange&& entities, AccessDefinitions& access_definitions, LoopBody loop_body) const
        {
            auto it = entities.begin();
            const std::size_t num_entities = entities.GetRangeSize();

            if constexpr (Policy == ExecutionPolicy::SIMD)
            {
                constexpr std::size_t ChunkSize = 8;
                
                for (std::size_t i = 0; i < num_entities; i += ChunkSize)
                {
                    const std::size_t ii_max = std::min(num_entities - i, ChunkSize);
                    auto&& entity{GetEntities(it, ii_max, std::make_index_sequence<ChunkSize>{})};
                    auto&& local_vectors = std::make_pair(LocalView::CreateMultiple(access_definitions, it, ii_max, std::make_index_sequence<ChunkSize>{}), ii_max);
                    
                    loop_body(entity, local_vectors);
                    it += ChunkSize;
                }
            }
            else
            {
                for (std::size_t i = 0; i < num_entities; ++i, ++it)
                {
                    auto&& localVector{LocalView::Create(access_definitions, *it)};

                    loop_body(*it, localVector);
                }
            }
        }
    };

    //! \brief
    //! The default implementation for the `ForEachIncidence` loop
    //!
    //! \tparam
    //! Dimension_ specifies the dimension of requested entities to iterate over
    //!
    //! \tparam
    //! SubDimension specifies the dimension of requested sub-entities to iterate over
    template <std::size_t Dimension_, std::size_t SubDimension, ExecutionPolicy Policy_>
    struct ForEachIncidence
    {
        //! Is used to infer the dimension of entities in operator() by the run time system.
        static constexpr std::size_t Dimension = Dimension_;

        static constexpr ExecutionPolicy Policy = Policy_;

        //! \brief Iterates over all given `entities` and its sub-entities of a given SubDimension and invokes `loop_body` for each sub-entity.
        //!
        //! For example, given a vector of edges `e` and the SubDimension for nodes, operator()(e, [](auto node) {}) iterates over all child-nodes of each edge in `e` and invokes `loop_body` for each of the nodes.
        //!
        //! \attention
        //! `loop_body` must be invocable with the requested sub-entities
        //!
        //! \note
        //! Since `loop_body` is invoked for each child-entity of each parent-entity in `entities`, the same sub-entity is called more than once with different parent-entities.
        //! For example, a node belongs to multiple edges and is therefore visited multiple times.
        template <typename EntityRange, typename LoopBody>
        auto operator()(EntityRange&& entities, LoopBody loop_body) const
        {
            for (auto const& entity : std::forward<EntityRange>(entities))
            {
                for (auto const& subEntity : entity.GetTopology().template GetEntities<SubDimension>())
                {
                    loop_body(subEntity);
                }
            }
        }

        //! \brief Iterates over all given `entities` and its sub-entities of a given SubDimension and invokes `loop_body` for each sub-entity. It further constructs the appropriate local vectors for the defined
        //! access definitions.
        //!
        //! For example, given a vector of edges `e` and the SubDimension for nodes, operator()(e, [](auto node, auto local_view) {}) iterates over all child-nodes of each edge in `e` and invokes `loop_body` for each
        //! of the nodes.
        //!
        //! \attention
        //! `loop_body` must be invocable with the requested sub-entities and local_vectors
        //!
        //! \note
        //! Since `loop_body` is invoked for each child-entity of each parent-entity in `entities`, the same sub-entity is called more than once with different parent-entities.
        //! For example, a node belongs to multiple edges and is therefore visited multiple times.
        //!
        //! \see
        //! AccessDefinition.hpp
        template <typename EntityRange, typename AccessDefinitions, typename LoopBody>
        auto operator()(EntityRange&& entities, AccessDefinitions& access_definitions, LoopBody loop_body) const
        {
            constexpr std::size_t NumSubEntities = EntityRange::EntityT::Topology::template GetNumEntities<SubDimension>();

            for (auto const& entity : std::forward<EntityRange>(entities))
            {
                auto it = entity.GetTopology().template GetEntities<SubDimension>().begin();
                auto&& local_vectors{LocalView::CreateMultiple(access_definitions, it, NumSubEntities, std::make_index_sequence<NumSubEntities>{})};

                for (std::size_t i = 0; i < NumSubEntities; ++i, ++it)
                {
                    loop_body(*it, local_vectors[i]);
                }
            }
        }
    };
} // namespace HPM::internal
//! \}

#endif