// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef THIRDPARTY_METIS_PARTITIONER_HPP
#define THIRDPARTY_METIS_PARTITIONER_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

extern "C"
{
#include "metis.h"
}

#include <HighPerMeshes/dsl/meshes/Partitioner.hpp>

namespace HPM::mesh
{
    // Forward declaration.
    template <typename>
    class Partitioner;

    //!
    //! \brief A partitioner type using METIS.
    //!
    class MetisPartitioner : public Partitioner<MetisPartitioner>
    {
        friend class Partitioner<MetisPartitioner>;

        using METISIndexType = idx_t;
        using METISRealType = real_t;

        //!
        //! \brief Create a partitioning of the elements.
        //!
        //! This implementation uses METIS for the partitioning.
        //! Each element consists of a set of nodes.
        //! The number of nodes shared by two neighboring elements must be specified for the partitioning.
        //!
        //! \tparam NumNodesPerElement the number of nodes per element
        //! \tparam NumCommonNodes the number of nodes two neighboring elements have in common
        //! \param elements a set of elements to be partitioned
        //! \param num_nodes the total number of nodes among all elements
        //! \param num_partitions the number of partitions to be created
        //! \return a tuple consisting of vector containers holding the mapping of elements to partitions and nodes to partitions
        //!
        template <std::size_t NumCommonNodes, std::size_t NumNodesPerElement>
        auto CreatePartitionImplementation(const std::vector<std::array<std::size_t, NumNodesPerElement>>& elements, const std::size_t num_nodes, const std::size_t num_partitions) const
        {
            const std::size_t num_elements = elements.size();

            // Set up an element pointer: succesive elements are separated by 'NumNodesPerElement' nodes.
            std::vector<METISIndexType> element_pointer(num_elements + 1);

            element_pointer[0] = 0;
            for (std::size_t element_index = 0; element_index < num_elements; ++element_index)
            {
                element_pointer[element_index + 1] = element_pointer[element_index] + NumNodesPerElement;
            }

            // Setup a node pointer: make a copy of the content of the 'elements' field.
            // This is a 2D to 1D copy, effectively.
            std::vector<METISIndexType> node_pointer(element_pointer[num_elements]);

            for (std::size_t element_index = 0; element_index < num_elements; ++element_index)
            {
                for (std::size_t node_index = 0; node_index < NumNodesPerElement; ++node_index)
                {
                    node_pointer[element_pointer[element_index] + node_index] = elements[element_index][node_index];
                }
            }

            // Allocate memory for the result of the partitioning.
            //    - a container that holds for each element the index of the partition it belongs to
            //    - a container that holds for each node the index of the partition it belongs to
            std::vector<METISIndexType> element_to_partition_pointer(num_elements);
            std::vector<METISIndexType> node_to_partition_pointer(num_nodes);

            // Do the partitioning: call METIS only if num_partitions > 1, otherwise METIS segfaults.
            if (num_partitions == 1)
            {
                for (std::size_t i = 0; i < num_elements; ++i)
                {
                    element_to_partition_pointer[i] = 0;
                }

                for (std::size_t i = 0; i < num_nodes; ++i)
                {
                    node_to_partition_pointer[i] = 0;
                }
            }
            else if (num_partitions > 1)
            {
                const METISIndexType n_elements = num_elements;
                const METISIndexType n_nodes = num_nodes;
                const METISIndexType n_partitions = num_partitions;
                // METIS options: use default, and switch on the sub-set of options needed.
                METISIndexType options[METIS_NOPTIONS];
                METIS_SetDefaultOptions(options);
                // C-style array numbering/indexing.
                options[METIS_OPTION_NUMBERING] = 0;
                // Edge-cut minimization, should be OK in this case.
                options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
                // Number of common primal mesh nodes needed to link two FEs by an edge in the dual graph (where the FEs are nodes).
                const METISIndexType n_common_nodes = NumCommonNodes;
                // Final estimate how good the partitioning is.
                METISIndexType objVal;

                switch (METIS_PartMeshDual(const_cast<METISIndexType*>(&n_elements),     // number FEs to partition
                                            const_cast<METISIndexType*>(&n_nodes),        // numbr nodes to partition
                                            element_pointer.data(),                       // elm pointers array
                                            node_pointer.data(),                          // elm indices array
                                            nullptr,                                      // == nullptr, array of FE-weights, equal weights
                                            nullptr,                                      // == nullptr, size of FE(s); equal for all FEs
                                            const_cast<METISIndexType*>(&n_common_nodes), // number of common vertices between FEs needed to "create" an edge between two FEs
                                            const_cast<METISIndexType*>(&n_partitions),   // number of partitions
                                            nullptr,                                      // == nullptr, evntl. different weights per SD
                                            options,                                      // the array of options
                                            &objVal,                                      // estimates how "good" the DD is
                                            element_to_partition_pointer.data(),          // elm partition array
                                            node_to_partition_pointer.data()))            // nodal partition arrray
                {
                case (METIS_OK):
                    break;
                case (METIS_ERROR_INPUT):
                    throw std::runtime_error("Fatal Error Metis DD: input error detected.");
                    break;
                case (METIS_ERROR_MEMORY):
                    throw std::runtime_error("Fatal Error Metis DD: could not allocate required memory.");
                    break;
                default:
                    throw std::runtime_error("Fatal Error Metis DD: unspecified error.");
                    break;
                }
            }

            // Mapping of elements and nodes to partitions.
            std::vector<std::size_t> element_to_partition(num_elements);
            std::vector<std::size_t> node_to_partition(num_nodes);

            for (std::size_t element_index = 0; element_index < num_elements; ++element_index)
            {
                element_to_partition[element_index] = element_to_partition_pointer[element_index];
            }

            for (std::size_t node_index = 0; node_index < num_nodes; ++node_index)
            {
                node_to_partition[node_index] = node_to_partition_pointer[node_index];
            }

            return std::make_tuple(std::move(element_to_partition), std::move(node_to_partition));
        }
    };
} // namespace HPM::mesh

#endif
