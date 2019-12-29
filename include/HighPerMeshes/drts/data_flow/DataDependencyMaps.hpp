/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2018
 *
 * This file is part of HighPerMeshesDRTS, the HighPerMeshes distributed runtime
 * system.
 *
 * The HighPerMeshesDRTS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * HighPerMeshesDRTS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HighPerMeshesDRTS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Graph.hpp
 *
 */

#ifndef DRTS_DATAFLOW_DATADEPENDENCYMAPS_HPP
#define DRTS_DATAFLOW_DATADEPENDENCYMAPS_HPP

#include <cstdint>
#include <map>

#include <HighPerMeshes/dsl/loop_types/loop_implementations/DefaultLoopImplementations.hpp>
#include <HighPerMeshes/dsl/meshes/PartitionedMesh.hpp>

namespace HPM::drts::data_flow
{
    //!
    //! \brief This class specifies the dependencies between two local (L2) partitions given an access pattern and a loop implementation.
    //!
    //! \tparam Dimension The dimension of the mesh this data dependency map is calculated for
    //!
    template <std::size_t Dimension>
    class DataDependencyMap
    {
        //! Specifies an entity by its index and codimension.
        struct EntitySpecification
        {
            const std::size_t index;
            const std::size_t codimension;
        };

        public:
        //!
        //! \param pattern An access pattern
        //! \param loop A loop implementation
        //!
        //! \see AccessPattern.hpp
        //! \see DefaultLoopImplementations.hpp
        //!
        template <typename MeshT, typename Pattern, typename LoopT>
        DataDependencyMap(const MeshT& mesh, const Pattern pattern, const LoopT loop)
        {
            auto detectL2Accesses = [&](const auto& entity, std::size_t L2) {
                const auto& considered_element = pattern(entity);
                using ConsideredElementT = std::decay_t<decltype(considered_element)>;
                constexpr std::size_t RequiredCodimension = ConsideredElementT::CellDimension - ConsideredElementT::Dimension;
                const std::size_t other_L2 = mesh.EntityToL2P(considered_element);
                const std::size_t other_index = considered_element.GetTopology().GetIndex();

                has_access[L2].insert(other_L2);
                has_access_by_entity[{L2, other_L2}][RequiredCodimension].insert(other_index);
            };

            for (std::size_t i_L1 = 0; i_L1 < mesh.GetNumL1Partitions(); ++i_L1)
            {
                for (auto L2 : mesh.L1PToL2P(i_L1))
                {
                    const auto& elements = mesh.template L2PToEntity<LoopT::Dimension>(L2);
                    loop(elements, [&](const auto& entity) { detectL2Accesses(entity, L2); });
                }
            }
        }

        //!
        //! \return All L2 partitions that `accessor_L2` has access to
        //!
        const auto& L2PHasAccessToL2P(const std::size_t accessor_L2) const
        {
            auto iter = has_access.find(accessor_L2);

            return (iter != has_access.end() ? iter->second : empty_access);
        }

        //!
        //! \return An array of indices that determines all entities that the L2 partition `accessor_L2` can access in partition `accessed_L2`.
        //!         The index given to the arrays operator[] determines the codimension of the entity and the value of the underlying sets specify the global index of the underlying entity.
        //!
        const auto& L2PHasAccessToL2PByEntity(const std::size_t accessor_L2, const std::size_t accessed_L2) const
        {
            auto iter = has_access_by_entity.find({accessor_L2, accessed_L2});
            return iter != has_access_by_entity.end() ? iter->second : empty_access_by_entity;
        }

        //! Adds all indices for all relationships in other to this.
        void operator+=(const DataDependencyMap<Dimension>&& other)
        {
            auto move_insert = [](auto&& dst, auto&& src) { dst.insert(make_move_iterator(src.begin()), make_move_iterator(src.end())); };

            for (auto [key, value] : other.has_access)
            {
                move_insert(has_access[key], value);
            }

            for (auto [key, value] : other.has_access_by_entity)
            {
                for (std::size_t dim = 0; dim < value.size(); ++dim)
                {
                    move_insert(has_access_by_entity[key][dim], value[dim]);
                }
            }
        }

        private:
        const std::set<std::size_t> empty_access;
        std::map<std::size_t, std::set<std::size_t>> has_access;
        const std::array<std::set<std::size_t>, Dimension + 1> empty_access_by_entity;
        std::map<std::pair<std::size_t, std::size_t>, std::array<std::set<std::size_t>, Dimension + 1>> has_access_by_entity;
    };
} // namespace HPM::drts::data_flow

#endif