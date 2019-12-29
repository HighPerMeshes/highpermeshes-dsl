// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_MESHES_PARTITIONER_HPP
#define DSL_MESHES_PARTITIONER_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace HPM::mesh
{
    //!
    //! \brief A partitioner type.
    //!
    //! This type provides a 2-level partitioning.
    //! Implementing (derived) classes must provide a `CreatePartitionImplementation`  member function that implements the partitioning
    //! of the elements using a specific number of nodes common to neighboring elements.
    //!
    //! \tparam Implementation the type of the implementing (derived) class (CRTP)
    //!
    template <typename Implementation>
    class Partitioner
    {
        protected:
        //!
        //! \brief Protected Constructor.
        //!
        //! Only derived classes can instantiate this type.
        //!
        Partitioner() = default;

        //!
        //! \brief Create a partitioning of the elements.
        //!
        //! \tparam NumNodesPerElement the number of nodes per element
        //! \tparam NumCommonNodes the number of nodes two neighboring elements have in common
        //! \param elements a set of elements to be partitioned
        //! \param num_nodes the total number of nodes among all elements
        //! \param num_partitions the number of partitions to be created
        //! \return a tuple consisting of vector containers holding the mapping of elements to partitions and nodes to partitions
        //!
        template <std::size_t NumCommonNodes, std::size_t NumNodesPerElement>
        auto CreatePartition(const std::vector<std::array<std::size_t, NumNodesPerElement>>& elements, const std::size_t num_nodes, const std::size_t num_partitions) const
        {
            return static_cast<const Implementation&>(*this).template CreatePartitionImplementation<NumCommonNodes>(elements, num_nodes, num_partitions);
        }

        public:
        //!
        //! \brief Create a partitioning of elements.
        //!
        //! This function implements a two-level partitioning of the elements.
        //! Furthermore, nodes and elements are ordered by their level-2 partiton.
        //! A mapping between the original node indices and the reordered ones is created.
        //!
        //! \tparam NumCommonNodes the number of nodes two neighboring elements have in common
        //! \tparam NumNodesPerElement the number of nodes per element
        //! \tparam CoordinateT the coordinate type used for the node (vertex) representation
        //! \param elements a set of elements to be partitioned
        //! \param num_nodes the total number of nodes among all elements
        //! \param num_partitions the number of level-1 (L1) and level-2 (L2) partitions to be created
        //! \return a tuple consisting of vector containers holding
        //!     the ordered nodes and elements,
        //!     the mapping of nodes and elements to level-2 partitions
        //!     offsets (prefix sums) for the number nodes and elements in level-2 partitions
        //!
        template <std::size_t NumCommonNodes, std::size_t NumNodesPerElement, typename CoordinateT>
        auto CreatePartitions(std::vector<CoordinateT>&& nodes, std::vector<std::array<std::size_t, NumNodesPerElement>>&& elements, const std::pair<std::size_t, std::size_t>& num_partitions)
        {
            const std::size_t num_nodes = nodes.size();
            const std::size_t num_elements = elements.size();
            const std::size_t num_L2_partitions(std::get<0>(num_partitions) * std::get<1>(num_partitions));
            std::vector<std::size_t> element_to_L2P(num_elements);
            std::vector<std::size_t> num_elements_in_L2P_offset(num_L2_partitions + 1);
            std::vector<std::size_t> node_to_L2P(num_nodes);
            std::vector<std::size_t> num_nodes_in_L2P_offset(num_L2_partitions + 1);

            // Do the first level (L1) partitioning.
            std::vector<std::size_t> element_to_L1P;
            std::vector<std::size_t> node_to_L1P;
            std::tie(element_to_L1P, node_to_L1P) = CreatePartition<NumCommonNodes>(elements, num_nodes, std::get<0>(num_partitions));

            // Order elements by their L1 partition: ordering within any L1 partition is implicit.
            std::vector<std::vector<std::size_t>> L1P_to_element(std::get<0>(num_partitions));

            for (std::size_t element_index = 0; element_index < element_to_L1P.size(); ++element_index)
            {
                L1P_to_element[element_to_L1P[element_index]].push_back(element_index);
            }

            // Do the second level (L2) partitioning: iterate over all L1 partitions.
            for (std::size_t i_L1 = 0; i_L1 < std::get<0>(num_partitions); ++i_L1)
            {
                const std::size_t num_elements_in_L1P = L1P_to_element[i_L1].size();

                // Collect all elements in this L1 partition.
                std::vector<std::array<std::size_t, NumNodesPerElement>> elements_in_L1P(num_elements_in_L1P);
                for (std::size_t element_index = 0; element_index < num_elements_in_L1P; ++element_index)
                {
                    elements_in_L1P[element_index] = elements[L1P_to_element[i_L1][element_index]];
                }

                // Now do the partitioning of this L1 partition: use only elements in this L1 partition and the whole set of nodes.
                std::vector<std::size_t> element_to_partition;
                std::vector<std::size_t> node_to_partition;
                std::tie(element_to_partition, node_to_partition) = CreatePartition<NumCommonNodes>(elements_in_L1P, num_nodes, std::get<1>(num_partitions));

                // Iterate over all the nodes..
                for (std::size_t node_index = 0; node_index < node_to_partition.size(); ++node_index)
                {
                    // ..and consider only those in this L1 partition.
                    if (node_to_L1P[node_index] == i_L1)
                    {
                        // Translate the local partition index into the L2 partition index: i_L1 * num_L1_partitions + local_partition_index.
                        node_to_L2P[node_index] = i_L1 * std::get<1>(num_partitions) + node_to_partition[node_index];
                    }
                }

                // For each element in this L1 partition translate its local partition index into the L2 partition index (similar to the node case).
                for (std::size_t element_index = 0; element_index < elements_in_L1P.size(); ++element_index)
                {
                    element_to_L2P[L1P_to_element[i_L1][element_index]] = i_L1 * std::get<1>(num_partitions) + element_to_partition[element_index];
                }
            }

            // At this point, we have the mapping of nodes and elements to their L2 partitions.

            // Order the nodes by their L2 partition and store the index remapping.
            std::vector<std::size_t> node_index_remapping(num_nodes);
            {
                std::vector<std::set<std::size_t>> L2P_to_node(num_L2_partitions);

                // Insert node indices into L2 partitions: ordered.
                for (std::size_t node_index = 0; node_index < num_nodes; ++node_index)
                {
                    L2P_to_node[node_to_L2P[node_index]].insert(node_index);
                }

                std::vector<CoordinateT> remapped_nodes(num_nodes);
                std::size_t i_L2 = 0;
                std::size_t remapped_node_index = 0;

                // For each L2 partition consider its node indices.
                num_nodes_in_L2P_offset[0] = 0;
                for (const auto& nodes_in_L2P : L2P_to_node)
                {
                    // For each node index in this L2 partition..
                    for (const auto& node_index : nodes_in_L2P)
                    {
                        // ..insert the node into the per-partition node list,
                        remapped_nodes[remapped_node_index] = nodes[node_index];
                        // ..note the partition this node belongs to,
                        node_to_L2P[remapped_node_index] = i_L2;
                        // ..and note the translation between the original and the remapped index.
                        node_index_remapping[node_index] = remapped_node_index;
                        // Go to the next node.
                        ++remapped_node_index;
                    }

                    // Go to the next partition and note the number of nodes in the L2 partitions already considered.
                    num_nodes_in_L2P_offset[++i_L2] = remapped_node_index;
                }

                // Replace the unordered node incides by the those ordered by their L2 partitions.
                nodes.swap(remapped_nodes);
            }

            // Order the elements by their L2 partition.
            {
                std::vector<std::set<std::size_t>> L2P_to_element(num_L2_partitions);

                // Insert element indices into L2 partitions: ordered.
                for (std::size_t element_index = 0; element_index < num_elements; ++element_index)
                {
                    L2P_to_element[element_to_L2P[element_index]].insert(element_index);
                }

                std::vector<std::array<std::size_t, NumNodesPerElement>> remapped_elements(num_elements);
                std::size_t i_L2 = 0;
                std::size_t remapped_element_index = 0;

                // For each L2 partition consider its element indices.
                num_elements_in_L2P_offset[0] = 0;
                for (const auto& elements_in_L2P : L2P_to_element)
                {
                    // For each element index in this L2 partition..
                    for (const auto& element_index : elements_in_L2P)
                    {
                        // ..translate its node indices using the remapped indexing,
                        for (std::size_t node_index = 0; node_index < NumNodesPerElement; ++node_index)
                        {
                            remapped_elements[remapped_element_index][node_index] = node_index_remapping[elements[element_index][node_index]];
                        }

                        // ..and note the partition this element belongs to.
                        element_to_L2P[remapped_element_index] = i_L2;
                        // Go to the next element.
                        ++remapped_element_index;
                    }

                    // Note the number of elements in the L2 partitions already considered.
                    num_elements_in_L2P_offset[i_L2 + 1] = remapped_element_index;

                    // Sort all elements in this L2 partition: mappings of elements into and from the L2 partions intact!
                    // Why sorting? Enable binary search for a specific element, given its node indices.
                    std::sort(remapped_elements.begin() + num_elements_in_L2P_offset[i_L2], remapped_elements.begin() + num_elements_in_L2P_offset[i_L2 + 1]);

                    // Go to the next partition.
                    ++i_L2;
                }

                // Replace the unordered elements by the those ordered by their L2 partitions.
                elements.swap(remapped_elements);
            }

            return std::make_tuple(std::move(nodes), std::move(elements), std::move(element_to_L2P), std::move(node_to_L2P), std::move(num_elements_in_L2P_offset), std::move(num_nodes_in_L2P_offset));
        }
    };

    //!
    //! \brief A simple partitioner type.
    //!
    class SimplePartitioner : public Partitioner<SimplePartitioner>
    {
        friend class Partitioner<SimplePartitioner>;

        //!
        //! \brief Create a partitioning of the elements.
        //!
        //! This implementation applies a 1:1 mapping: only 1 partition is supported.
        //!
        //! \tparam NumNodesPerElement the number of nodes per element
        //! \tparam NumCommonNodes the number of nodes two neighboring elements have in common
        //! \param elements a set of elements to be partitioned
        //! \param num_nodes the total number of nodes among all elements
        //! \param num_partitions the number of partitions to be created (always 1)
        //! \return a tuple consisting of vector containers holding the mapping of elements to partitions and nodes to partitions
        //!
        template <std::size_t NumCommonNodes, std::size_t NumNodesPerElement>
        auto CreatePartitionImplementation(const std::vector<std::array<std::size_t, NumNodesPerElement>>& elements, const std::size_t num_nodes, const std::size_t num_partitions) const
        {
            if (num_partitions > 1)
            {
                throw std::runtime_error("SimplePartitioner::CreatePartition: multiple partitions are not supported.");
            }

            // Mapping of elements and nodes to partitions: only 1 partition is supported!
            const std::size_t num_elements = elements.size();
            std::vector<std::size_t> element_to_partition(num_elements);
            std::vector<std::size_t> node_to_partition(num_nodes);

            for (std::size_t element_index = 0; element_index < num_elements; ++element_index)
            {
                element_to_partition[element_index] = 0;
            }

            for (std::size_t node_index = 0; node_index < num_nodes; ++node_index)
            {
                node_to_partition[node_index] = 0;
            }

            return std::make_tuple(std::move(element_to_partition), std::move(node_to_partition));
        }
    };
} // namespace HPM::mesh

#endif