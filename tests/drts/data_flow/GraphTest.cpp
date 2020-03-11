// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

#include <HighPerMeshes/drts/data_flow/Graph.hpp>
#include <sstream>

extern int numThreads;

using namespace HPM;
using namespace HPM::drts::data_flow;

class GraphTest : public ::testing::Test
{
  public:
    using TestType = std::string;
    Graph<TestType> graph;

    TestType field{"field"};

    size_t countDependencies(size_t vertex) const
    {
        const std::vector<Graph<TestType>::Edge>& graphDependencies = graph.GetEdges();
        return std::count_if(graphDependencies.begin(), graphDependencies.end(), [vertex](auto dependency) { return dependency.consumer == vertex; });
    }

    auto countDependencies() const { 
        const std::vector<Graph<TestType>::Edge>& graphDependencies = graph.GetEdges();
        return graphDependencies.size(); 
    }

    auto dependencies(size_t vertex) const
    {
        const std::vector<Graph<TestType>::Edge>& graphDependencies = graph.GetEdges();
        std::vector<Graph<TestType>::Edge> filtered;
        std::copy_if(graphDependencies.begin(), graphDependencies.end(), std::back_inserter(filtered), [vertex](auto dependency) { return dependency.consumer == vertex; });
        return filtered;
    }

    auto firstDependency(size_t vertex) const { return dependencies(vertex)[0]; }

    auto size() const { return graph.GetVertices().size(); }
};

TEST_F(GraphTest, singleNodeGraph)
{
    auto loopVertex = graph.AddVertex();
    graph.AddDependency(loopVertex, field, AccessMode::ReadWrite);
    graph.Finalize();

    EXPECT_EQ(size(), 1);
}

TEST_F(GraphTest, SolvedIsolatedInput)
{

    auto loop = graph.AddVertex();
    graph.AddDependency(loop, field, AccessMode::Read);
    graph.Finalize();

    EXPECT_EQ(size(), 2);
    EXPECT_EQ(countDependencies(), 1);
    EXPECT_EQ(countDependencies(loop), 1);

    const auto& dependency = firstDependency(loop);
    const auto producer = dependency.producer;
    EXPECT_EQ(dependency.consumer, loop);
    EXPECT_EQ(countDependencies(producer), 0);

    EXPECT_LT(dependency.producer, dependency.consumer);
}

TEST_F(GraphTest, SolvedIsolatedOutput)
{

    auto loop = graph.AddVertex();
    graph.AddDependency(loop, field, AccessMode::Write);
    graph.Finalize();

    EXPECT_EQ(size(), 2);
    EXPECT_EQ(countDependencies(), 1);
    EXPECT_EQ(countDependencies(loop), 0);

    /**
     * Getting the new node is awkward. This is fine since we only
     * need to iterate over all dependencies in our application and do not need proper
     * random access.
     */
    EXPECT_EQ(countDependencies(), 1);
    for (const auto vertex : graph.GetVertices())
    {
        if (vertex == loop)
            continue;
        EXPECT_NE(vertex, loop);
        EXPECT_EQ(countDependencies(vertex), 1);
        EXPECT_EQ(firstDependency(vertex).consumer, vertex);
        EXPECT_EQ(firstDependency(vertex).producer, loop);
        EXPECT_LT(firstDependency(vertex).producer, firstDependency(vertex).consumer);
    }
}

TEST_F(GraphTest, midgThreeKernelGraph)
{
    TestType field2("field2");
    TestType field3("field3");

    auto VK = graph.AddVertex();
    graph.AddDependency(VK, field, AccessMode::Read);
    graph.AddDependency(VK, field2, AccessMode::Write);

    auto SK = graph.AddVertex();
    graph.AddDependency(SK, field, AccessMode::Read);
    graph.AddDependency(SK, field2, AccessMode::ReadWrite);

    auto RK = graph.AddVertex();
    graph.AddDependency(RK, field, AccessMode::ReadWrite);
    graph.AddDependency(RK, field2, AccessMode::Read);
    graph.AddDependency(RK, field3, AccessMode::ReadWrite);

    graph.Finalize();

    EXPECT_EQ(size(), 3);
    EXPECT_EQ(countDependencies(), 6);

    // check correct dependencies of VK loop
    {
        EXPECT_EQ(countDependencies(VK), 1);
        auto dependency = firstDependency(VK);

        EXPECT_EQ(dependency.consumer, VK);
        EXPECT_EQ(dependency.edge, field);
        EXPECT_EQ(dependency.producer, RK);
    }
    // check correct dependencies of SK loop
    {
        EXPECT_EQ(countDependencies(SK), 2);

        for (const auto& dependency : dependencies(SK))
        {

            if (dependency.edge == field)
            {

                EXPECT_EQ(dependency.consumer, SK);
                EXPECT_EQ(dependency.producer, RK);
            }
            else if (dependency.edge == field2)
            {

                EXPECT_EQ(dependency.consumer, SK);
                EXPECT_EQ(dependency.producer, VK);
            }
            else
            {
                FAIL() << "Unexpected field type";
            }
        }
    }

    // check correct dependencies of RK loop
    {
        EXPECT_EQ(countDependencies(RK), 3);

        for (const auto& dependency : dependencies(RK))
        {

            if (dependency.edge == field)
            {

                EXPECT_EQ(dependency.consumer, RK);
                EXPECT_EQ(dependency.producer, RK);
            }
            else if (dependency.edge == field2)
            {

                EXPECT_EQ(dependency.consumer, RK);
                EXPECT_EQ(dependency.producer, SK);
            }
            else if (dependency.edge == field3)
            {

                EXPECT_EQ(dependency.consumer, RK);
                EXPECT_EQ(dependency.producer, RK);
            }
            else
            {
                throw std::runtime_error("Unexpected field type");
            }
        }
    }
}

TEST_F(GraphTest, kaskadeAssemblyGraph)
{
    auto loop = graph.AddVertex();
    graph.AddDependency(loop, field, AccessMode::Accumulate);
    graph.Finalize();

    EXPECT_EQ(size(), 2);

    /**
     * Getting the new node is awkward. This is fine since we only
     * need to iterate over all dependencies in our application and do not need proper
     * random access to vertices.
     */
    EXPECT_EQ(countDependencies(), 2);
    size_t newNode = -1;
    for (const auto dependency : graph.GetEdges())
    {
        if (dependency.consumer != loop)
        {
            newNode = dependency.consumer;
        }
    }

    // check correct dependencies of ForEachIntersection loop
    EXPECT_EQ(countDependencies(loop), 1);

    EXPECT_EQ(firstDependency(loop).consumer, loop);
    EXPECT_EQ(firstDependency(loop).edge, field);
    EXPECT_EQ(firstDependency(loop).producer, newNode);

    EXPECT_EQ(countDependencies(newNode), 1);

    EXPECT_EQ(firstDependency(newNode).consumer, newNode);
    EXPECT_EQ(firstDependency(newNode).edge, field);
    EXPECT_EQ(firstDependency(newNode).producer, loop);
}

TEST_F(GraphTest, WriteToRead)
{
    // A -> B
    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::Write);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::Read);

    graph.Finalize();

    EXPECT_EQ(countDependencies(), 1);
    EXPECT_EQ(countDependencies(A), 0);
    EXPECT_EQ(countDependencies(B), 1);

    // A -> B
    EXPECT_EQ(firstDependency(B).producer, A);
    EXPECT_EQ(firstDependency(B).consumer, B);
}

TEST_F(GraphTest, ReadToWrite)
{
    // A <- B
    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::Read);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::Write);

    graph.Finalize();

    EXPECT_EQ(countDependencies(), 1);
    EXPECT_EQ(countDependencies(A), 1);
    EXPECT_EQ(countDependencies(B), 0);

    // A <- B
    EXPECT_EQ(firstDependency(A).producer, B);
    EXPECT_EQ(firstDependency(A).consumer, A);
}

TEST_F(GraphTest, ReadWriteToReadWrite)
{
    // A <-> B
    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::ReadWrite);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::ReadWrite);

    graph.Finalize();

    EXPECT_EQ(countDependencies(), 2);
    EXPECT_EQ(countDependencies(A), 1);
    EXPECT_EQ(countDependencies(B), 1);

    // A <- B
    EXPECT_EQ(firstDependency(A).producer, B);
    EXPECT_EQ(firstDependency(A).consumer, A);

    // A -> B
    EXPECT_EQ(firstDependency(B).producer, A);
    EXPECT_EQ(firstDependency(B).consumer, B);
}

TEST_F(GraphTest, IndirectDependency)
{
    // A -> B -> C
    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::Write);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::ReadWrite);

    auto C = graph.AddVertex();
    graph.AddDependency(C, field, AccessMode::Read);

    graph.Finalize();

    EXPECT_EQ(countDependencies(), 2);
    EXPECT_EQ(countDependencies(A), 0);
    EXPECT_EQ(countDependencies(B), 1);
    EXPECT_EQ(countDependencies(C), 1);

    // A -> B
    EXPECT_EQ(firstDependency(B).producer, A);
    EXPECT_EQ(firstDependency(B).consumer, B);

    // B -> C
    EXPECT_EQ(firstDependency(C).producer, B);
    EXPECT_EQ(firstDependency(C).consumer, C);
}

TEST_F(GraphTest, CyclicDependency)
{
    // A <-> B <-> C
    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::ReadWrite);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::ReadWrite);

    auto C = graph.AddVertex();
    graph.AddDependency(C, field, AccessMode::ReadWrite);

    graph.Finalize();

    EXPECT_EQ(countDependencies(), 3);
    EXPECT_EQ(countDependencies(A), 1);
    EXPECT_EQ(countDependencies(B), 1);
    EXPECT_EQ(countDependencies(C), 1);

    // C -> A
    EXPECT_EQ(firstDependency(A).producer, C);
    EXPECT_EQ(firstDependency(A).consumer, A);

    // A -> B
    EXPECT_EQ(firstDependency(B).producer, A);
    EXPECT_EQ(firstDependency(B).consumer, B);

    // B -> C
    EXPECT_EQ(firstDependency(C).producer, B);
    EXPECT_EQ(firstDependency(C).consumer, C);
}

TEST_F(GraphTest, AddTwice)
{

    auto A = graph.AddVertex();
    graph.AddDependency(A, field, AccessMode::Write);
    graph.AddDependency(A, field, AccessMode::Write);

    auto B = graph.AddVertex();
    graph.AddDependency(B, field, AccessMode::Read);
    graph.AddDependency(B, field, AccessMode::Read);

    graph.Finalize();

    // The same dependencies should not be added twice to the graph
    EXPECT_EQ(countDependencies(), 1);
    EXPECT_EQ(countDependencies(B), 1);
}
