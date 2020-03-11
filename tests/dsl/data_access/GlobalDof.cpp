// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <cmath>
#include <iostream>
#include <utility>

#include <gtest/gtest.h>

#include <HighPerMeshes.hpp>

#include "../../util/Grid.hpp"

template <std::size_t Dimension>
class GlobalDofTest : public ::testing::Test
{
    auto GridExtent()
        -> std::array<std::size_t, Dimension>
    {
        if constexpr (Dimension == 2)
        {
            return {2, 2};
        }
        else if constexpr (Dimension == 3)
        {
            return {2, 2, 2};
        }

        return {};
    }

public:
    using CoordinateT = typename Grid<Dimension>::CoordinateT;
    using MeshT = ::HPM::mesh::PartitionedMesh<CoordinateT, ::HPM::entity::Simplex, Dimension>;

    GlobalDofTest()
        :
        grid(GridExtent()),
        mesh(std::vector<CoordinateT>(grid.nodes), std::vector<std::array<std::size_t, Dimension + 1>>(grid.simplices))
    {}

    ~GlobalDofTest() {}

    const auto& GetMesh() const { return mesh; }

protected:
    const Grid<Dimension> grid;
    MeshT mesh;
};

template <std::size_t Dimension, typename EntityT>
auto GetSmallestSubEntityIndex(const EntityT& entity)
{
    static_assert(Dimension <= EntityT::Dimension, "error: entity dimension must be larger than sub-entity dimension.");

    std::pair<std::size_t, std::size_t> index{EntityT::MeshT::InvalidIndex, 0};

    for (const auto& sub_entity : entity.GetTopology().template GetEntities<Dimension>())
    {
        const std::size_t sub_entity_index = sub_entity.GetTopology().GetIndex();

        if (sub_entity_index < index.first)
        {
            index.first = sub_entity_index;
            index.second = sub_entity.GetTopology().GetLocalIndex();
        }
    }

    return index;
}


using GlobalBufferTest_2d = GlobalDofTest<2>;
using GlobalBufferTest_3d = GlobalDofTest<3>;

TEST_F(GlobalBufferTest_2d, Pointers)
{
    using namespace ::HPM;

    drts::Runtime hpm{GetBuffer{}};
    constexpr auto Dofs = dof::MakeDofs<4, 2, 3, 7>();
    const auto& mesh = GetMesh();
    auto field{hpm.GetBuffer<CoordinateT>(mesh, Dofs)}; 

    SequentialDispatcher body;
    auto AllCells{mesh.GetEntityRange<2>()};
    std::array<std::ptrdiff_t, 3> offset;
    std::array<std::size_t, 3> expected_offset;

    expected_offset[2] = 7;
    expected_offset[1] = expected_offset[2] + Dofs.template At<2>() * mesh.template GetNumEntities<2>();
    expected_offset[0] = expected_offset[1] + Dofs.template At<1>() * mesh.template GetNumEntities<1>();

    body.Execute(
        ForEachEntity(
        AllCells,
        std::tuple(Read(Node(field)), Read(Edge(field)), Read(Cell(field)), Write(Global(field))),
        [&] (const auto& cell, auto&&, auto lvs)
        {
            const auto& field_node = dof::GetDofs<dof::Name::Node>(std::get<0>(lvs));
            const auto& field_edge = dof::GetDofs<dof::Name::Edge>(std::get<1>(lvs));
            const auto& field_cell = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs));
            auto& field_global = dof::GetDofs<dof::Name::Global>(std::get<3>(lvs));

            const auto& node_id = GetSmallestSubEntityIndex<0>(cell);
            const auto& edge_id = GetSmallestSubEntityIndex<1>(cell);
            const std::size_t cell_id = cell.GetTopology().GetIndex();

            field_global[cell_id][0] = 1 + cell_id;

            if (node_id.first == 0) offset[0] = &field_node[node_id.second][0] - &field_global[0];
            if (edge_id.first == 0) offset[1] = &field_edge[edge_id.second][0] - &field_global[0];
            if (cell_id == 0) offset[2] = &field_cell[0] - &field_global[0];
        })
    );

    EXPECT_EQ(offset[0], expected_offset[0]);
    EXPECT_EQ(offset[1], expected_offset[1]);
    EXPECT_EQ(offset[2], expected_offset[2]);

    const bool expected_assignment = [&field, &mesh] () 
        { 
            for (std::size_t i = 0; i < mesh.GetNumEntities(); ++i) 
                if (field[i][0] != (1 + i)) return false;
            return true; 
        } ();
    EXPECT_EQ(true, expected_assignment);
}

TEST_F(GlobalBufferTest_3d, Pointers)
{
    using namespace ::HPM;

    drts::Runtime hpm{GetBuffer{}};
    constexpr auto Dofs = dof::MakeDofs<4, 2, 3, 7, 6>();
    const auto& mesh = GetMesh();
    auto field{hpm.GetBuffer<CoordinateT>(mesh, Dofs)}; 

    SequentialDispatcher body;
    auto AllCells{mesh.GetEntityRange<3>()};
    std::array<std::ptrdiff_t, 4> offset;
    std::array<std::size_t, 4> expected_offset;

    expected_offset[3] = 6;
    expected_offset[2] = expected_offset[3] + Dofs.template At<3>() * mesh.template GetNumEntities<3>();
    expected_offset[1] = expected_offset[2] + Dofs.template At<2>() * mesh.template GetNumEntities<2>();
    expected_offset[0] = expected_offset[1] + Dofs.template At<1>() * mesh.template GetNumEntities<1>();

    body.Execute(
        ForEachEntity(
        AllCells,
        std::tuple(Read(Node(field)), Read(Edge(field)), Read(Face(field)), Read(Cell(field)), Write(Global(field))),
        [&] (const auto& cell, auto&&, auto lvs)
        {
            const auto& field_node = dof::GetDofs<dof::Name::Node>(std::get<0>(lvs));
            const auto& field_edge = dof::GetDofs<dof::Name::Edge>(std::get<1>(lvs));
            const auto& field_face = dof::GetDofs<dof::Name::Face>(std::get<2>(lvs));
            const auto& field_cell = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs));
            auto& field_global = dof::GetDofs<dof::Name::Global>(std::get<4>(lvs));

            const auto& node_id = GetSmallestSubEntityIndex<0>(cell);
            const auto& edge_id = GetSmallestSubEntityIndex<1>(cell);
            const auto& face_id = GetSmallestSubEntityIndex<2>(cell);
            const std::size_t cell_id = cell.GetTopology().GetIndex();

            field_global[cell_id][0] = 1 + cell_id;

            if (node_id.first == 0) offset[0] = &field_node[node_id.second][0] - &field_global[0];
            if (edge_id.first == 0) offset[1] = &field_edge[edge_id.second][0] - &field_global[0];
            if (face_id.first == 0) offset[2] = &field_face[face_id.second][0] - &field_global[0];
            if (cell_id == 0) offset[3] = &field_cell[0] - &field_global[0];
        })
    );

    EXPECT_EQ(offset[0], expected_offset[0]);
    EXPECT_EQ(offset[1], expected_offset[1]);
    EXPECT_EQ(offset[2], expected_offset[2]);
    EXPECT_EQ(offset[3], expected_offset[3]);

    const bool expected_assignment = [&field, &mesh] () 
        { 
            for (std::size_t i = 0; i < mesh.GetNumEntities(); ++i) 
                if (field[i][0] != (1 + i)) return false;
            return true; 
        } ();
    EXPECT_EQ(true, expected_assignment);
}