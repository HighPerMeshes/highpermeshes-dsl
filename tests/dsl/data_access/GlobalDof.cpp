// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <iostream>

#include <gtest/gtest.h>
/*
#include <HighPerMeshes/common/DataTypes.hpp>
#include <HighPerMeshes/drts/Runtime.hpp>
#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/PartitionedMesh.hpp>
*/
#include <HighPerMeshes.hpp>

#include "../../util/Grid.hpp"

template <std::size_t Dimension>
auto GridExtent()
    -> std::array<std::size_t, Dimension>
{
    if constexpr (Dimension == 1)
    {
        return {2};
    }
    else if constexpr (Dimension == 2)
    {
        return {2, 2};
    }
    else if constexpr (Dimension == 3)
    {
        return {2, 2, 2};
    }

    return {};
}

template <std::size_t Dimension>
class GlobalDofTest : public ::testing::Test
{
public:
    using CoordinateT = typename Grid<Dimension>::CoordinateT;
    using MeshT = ::HPM::mesh::PartitionedMesh<CoordinateT, ::HPM::entity::Simplex, Dimension>;

    GlobalDofTest()
        :
        grid(GridExtent<Dimension>()),
        mesh(std::vector<CoordinateT>(grid.nodes), std::vector<std::array<std::size_t, Dimension + 1>>(grid.simplices))
    {}

    ~GlobalDofTest() {}

    const auto& GetMesh() const { return mesh; }

protected:
    const Grid<Dimension> grid;
    MeshT mesh;
};

using GlobalBufferTest_3d = GlobalDofTest<3>;

TEST_F(GlobalBufferTest_3d, Pointers)
{
    ::HPM::drts::Runtime hpm{::HPM::GetBuffer{}};
    constexpr auto Dofs = ::HPM::dof::MakeDofs<1, 2, 3, 4>();
    auto field{hpm.GetBuffer<CoordinateT>(GetMesh(), Dofs)}; 
}