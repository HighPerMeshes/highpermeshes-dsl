// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

#include "../../util/Grid.hpp"
#include "../../util/UnitCube.hpp"

#include <HighPerMeshes/dsl/meshes/PartitionedMesh.hpp>
#include <HighPerMeshes/third_party/metis/Partitioner.hpp>

using PartitionedMesh = HPM::mesh::PartitionedMesh<GridCoordinates, HPM::entity::Simplex>;
using Partitioner = HPM::mesh::MetisPartitioner;

template <size_t D>
using Dimension = std::integral_constant<size_t, D>;

TEST(PartitionedMesh, BasicTest)
{

    Grid grid{9, 9};
    auto numL1andL2 = std::pair{1, 1};

    auto nodes = grid.nodes;
    auto simplexes = grid.simplexes;

    PartitionedMesh mesh{std::move(nodes), std::move(simplexes), numL1andL2, 0, Partitioner{}};

    auto EntityToL2P = [&mesh](size_t L2, auto Dimension) {
        for (auto entity : mesh.template L2PToEntity<Dimension.value>(L2))
        {
            ASSERT_EQ(mesh.template EntityToL2P<Dimension.value>(entity.GetTopology().GetIndex()), L2);
        }
    };

    // Check correct partition size calculation
    ASSERT_EQ(mesh.GetNumL1Partitions(), 1);
    ASSERT_EQ(mesh.GetNumL2Partitions(), 1);

    // Check correct mapping from L2 -> L1
    ASSERT_EQ(mesh.L2PToL1P(0), 0);

    // Check correct mapping from L1 -> L2: consecutive increment,
    // e.g. with 2 L1 partitions = (0, 1) and 4 L2 partitions = (0,1,2,3):
    // L2 = 0, 1 belong to L1 = 0
    // L2 = 2, 3 belong to L1 = 1
    for (auto L1 : mesh.L1PToL2P(0))
    {
        ASSERT_EQ(L1, 0);
    }

    // Check correct mapping between entity <-> L2:
    // mesh.L2PToEntity produces some amount of entities `e` that belong to some L2 partition `l2`.
    // e.EntityToL2p should therefore produce `l2`.
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 2>{});
}

TEST(PartitionedMesh, TwoL1_OneL2)
{

    Grid grid{9, 9};
    auto numL1andL2 = std::pair{2, 1};

    auto nodes = grid.nodes;
    auto simplexes = grid.simplexes;

    PartitionedMesh mesh{std::move(nodes), std::move(simplexes), numL1andL2, 0, Partitioner{}};

    auto EntityToL2P = [&mesh](size_t L2, auto Dimension) {
        for (auto entity : mesh.template L2PToEntity<Dimension.value>(L2))
        {
            ASSERT_EQ(mesh.template EntityToL2P<Dimension.value>(entity.GetTopology().GetIndex()), L2);
        }
    };

    // Check correct partition size calculation
    ASSERT_EQ(mesh.GetNumL1Partitions(), 2);
    ASSERT_EQ(mesh.GetNumL2Partitions(), 2);

    // Check correct mapping from L2 -> L1
    ASSERT_EQ(mesh.L2PToL1P(0), 0);
    ASSERT_EQ(mesh.L2PToL1P(1), 1);

    // Check correct mapping from L1 -> L2: consecutive increment,
    // e.g. with 2 L1 partitions = (0, 1) and 4 L2 partitions = (0,1,2,3):
    // L2 = 0, 1 belong to L1 = 0
    // L2 = 2, 3 belong to L1 = 1
    for (auto L1 : mesh.L1PToL2P(0))
    {
        ASSERT_EQ(L1, 0);
    }
    for (auto L1 : mesh.L1PToL2P(1))
    {
        ASSERT_EQ(L1, 1);
    }

    // Check correct mapping between entity <-> L2:
    // mesh.L2PToEntity produces some amount of entities `e` that belong to some L2 partition `l2`.
    // e.EntityToL2p should therefore produce `l2`.
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 2>{});

    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 2>{});
}

TEST(PartitionedMesh, OneL1_TwoL2)
{

    Grid grid{9, 9};
    auto numL1andL2 = std::pair{1, 2};

    auto nodes = grid.nodes;
    auto simplexes = grid.simplexes;

    PartitionedMesh mesh{std::move(nodes), std::move(simplexes), numL1andL2, 0, Partitioner{}};

    // Check correct partition size calculation
    ASSERT_EQ(mesh.GetNumL1Partitions(), 1);
    ASSERT_EQ(mesh.GetNumL2Partitions(), 2);

    // Check correct mapping from L2 -> L1
    ASSERT_EQ(mesh.L2PToL1P(0), 0);
    ASSERT_EQ(mesh.L2PToL1P(1), 0);

    // Check correct mapping from L1 -> L2: consecutive increment,
    // e.g. with 2 L1 partitions = (0, 1) and 4 L2 partitions = (0,1,2,3):
    // L2 = 0, 1 belong to L1 = 0
    // L2 = 2, 3 belong to L1 = 1
    size_t counter = 0;
    for (auto L1 : mesh.L1PToL2P(0))
    {
        ASSERT_EQ(L1, counter++);
    }

    auto EntityToL2P = [&mesh](size_t L2, auto Dimension) {
        for (auto entity : mesh.template L2PToEntity<Dimension.value>(L2))
        {
            ASSERT_EQ(mesh.template EntityToL2P<Dimension.value>(entity.GetTopology().GetIndex()), L2);
        }
    };

    // Check correct mapping between entity <-> L2:
    // mesh.L2PToEntity produces some amount of entities `e` that belong to some L2 partition `l2`.
    // e.EntityToL2p should therefore produce `l2`.
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 2>{});

    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 2>{});
}

TEST(PartitionedMesh, TwoL1_TwoL2)
{

    Grid grid{9, 9};
    auto numL1andL2 = std::pair{2, 2};

    auto nodes = grid.nodes;
    auto simplexes = grid.simplexes;

    PartitionedMesh mesh{std::move(nodes), std::move(simplexes), numL1andL2, 0, Partitioner{}};

    // Check correct partition size calculation
    ASSERT_EQ(mesh.GetNumL1Partitions(), 2);
    ASSERT_EQ(mesh.GetNumL2Partitions(), 4);

    // Check correct mapping from L2 -> L1
    ASSERT_EQ(mesh.L2PToL1P(0), 0);
    ASSERT_EQ(mesh.L2PToL1P(1), 0);
    ASSERT_EQ(mesh.L2PToL1P(2), 1);
    ASSERT_EQ(mesh.L2PToL1P(3), 1);

    // Check correct mapping from L1 -> L2: consecutive increment,
    // e.g. with 2 L1 partitions = (0, 1) and 4 L2 partitions = (0,1,2,3):
    // L2 = 0, 1 belong to L1 = 0
    // L2 = 2, 3 belong to L1 = 1
    size_t counter = 0;
    for (auto L1 : mesh.L1PToL2P(0))
    {
        ASSERT_EQ(L1, counter++);
    }
    for (auto L1 : mesh.L1PToL2P(1))
    {
        ASSERT_EQ(L1, counter++);
    }

    auto EntityToL2P = [&mesh](size_t L2, auto Dimension) {
        for (auto entity : mesh.template L2PToEntity<Dimension.value>(L2))
        {
            ASSERT_EQ(mesh.template EntityToL2P<Dimension.value>(entity.GetTopology().GetIndex()), L2);
        }
    };

    // Check correct mapping between entity <-> L2:
    // mesh.L2PToEntity produces some amount of entities `e` that belong to some L2 partition `l2`.
    // e.EntityToL2p should therefore produce `l2`.
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(0, Dimension<PartitionedMesh::CellDimension - 2>{});

    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 1>{});
    EntityToL2P(1, Dimension<PartitionedMesh::CellDimension - 2>{});
}
