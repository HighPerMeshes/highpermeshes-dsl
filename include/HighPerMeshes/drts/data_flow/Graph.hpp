// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DRTS_DATAFLOW_GRAPH
#define DRTS_DATAFLOW_GRAPH

#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

#include <HighPerMeshes/dsl/data_access/AccessMode.hpp>

namespace HPM::drts::data_flow
{
    //!
    //! \tparam Dependency must be able to uniquely identify a given dependency
    //!
    template <typename Dependency>
    class Graph
    {
        //!  Internal structure to store reads and writes for a given dependency
        struct ReadsAndWrites
        {
            std::set<std::size_t> reads;
            std::set<std::size_t> writes;

            //! Adds an access mode to a given vertex
            void Add(std::size_t vertex, AccessMode mode)
            {
                if (mode == AccessMode::Read || mode == AccessMode::ReadWrite)
                {
                    reads.insert(vertex);
                }

                if (mode == AccessMode::Write || mode == AccessMode::ReadWrite)
                {
                    writes.insert(vertex);
                }
            }
        };

        void AddVertex(const std::size_t id) { vertices.insert(id); }

        public:
        //! The Edge class consists of a producer and consumer node, identified by an integer and a dependency.
        struct Edge
        {
            Edge(const std::size_t producer, const Dependency edge, const std::size_t consumer) : producer(producer), edge(edge), consumer(consumer) {}

            const std::size_t producer;
            const Dependency edge;
            const std::size_t consumer;
        };

        //!
        //! \brief Constructor.
        //!
        Graph() : finalized {false}, initial_id {0}, current_id {1} {}

        //!
        //! \return a vertex, represented by an id, of the new node added to the graph.
        //!
        auto AddVertex()
        {
            assert(!finalized);

            AddVertex(current_id);

            return current_id++;
        }

        //!
        //! AddDependency prepares adding an edge between to nodes.
        //! During the finalize step, an edge between two vertices a and b is built if a is the latest vertex before / equal to b that writes to a `dependency` that b also reads from.
        //!
        void AddDependency(const std::size_t vertex, const Dependency dependency, const AccessMode mode)
        {
            assert(!finalized);

            auto& accesses = dependency_to_accesses.emplace(dependency, ReadsAndWrites{}).first->second;

            if (mode != AccessMode::Accumulate)
            {
                accesses.Add(vertex, mode);
            }
            else
            {
                // \todo {I'm not sure what accumulate is supposed to do. This is just in accordance with the old implementation - Stefan G. 12.8.2019}
                auto accumulationVertex = AddVertex();

                AddDependency(vertex, dependency, AccessMode::Read);
                AddDependency(vertex, dependency, AccessMode::Write);
                AddDependency(accumulationVertex, dependency, AccessMode::Read);
                AddDependency(accumulationVertex, dependency, AccessMode::Write);
            }
        }

        //!
        //!`finalize` constructs the actual edges between nodes after the last node has been added.
        //! A dependency is stored in the consumer node `n` for a dependency `d`. The corresponding producer is the
        //! first node that shares `d` as an initial dependency that comes before `n`. If the start of the nodes
        //! is reached we start from the end of the collection.
        //! If there are unmet read or write accesses we add another node to the front or back of the execution order
        //!
        void Finalize()
        {
            assert(!finalized);

            // If there are unmet read or write accesses we add another node to the front or back of the execution order
            for (const auto& [edge, reads_and_writes] : dependency_to_accesses)
            {
                if (reads_and_writes.reads.empty())
                {
                    AddVertex(current_id);
                    AddDependency(current_id, edge, AccessMode::Read);
                }
                if (reads_and_writes.writes.empty())
                {
                    AddVertex(initial_id);
                    AddDependency(initial_id, edge, AccessMode::Write);
                }
            }

            // Go forward through all initial edges. For such an edge go backward through the initial edges
            // until another initial edge e2 is found such that e.dependency == e2.dependency.
            for (const auto& [dependency, accesses] : dependency_to_accesses)
            {
                for (const auto& reader : accesses.reads)
                {

                    auto lower_bound = accesses.writes.lower_bound(reader);
                    // Note that lower_bound finds the element one larger than lower_bound, therefore we have to decrement it by one.
                    auto write_before = (lower_bound != accesses.writes.begin()) ? *(--lower_bound) : *(--accesses.writes.end());

                    edges.emplace_back(write_before, dependency, reader);
                }
            }

            finalized = true;
        }

        const auto& GetVertices() const
        {
            assert(finalized);

            return vertices;
        }

        const auto& GetEdges() const
        {
            assert(finalized);

            return edges;
        }

    private:
        bool finalized;
        std::size_t initial_id;
        std::size_t current_id;
        std::set<std::size_t> vertices;
        std::vector<Edge> edges;
        std::map<Dependency, ReadsAndWrites> dependency_to_accesses;
    };
} // namespace HPM::drts::data_flow

#endif