// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef TEST_CUBE_HPP
#define TEST_CUBE_HPP

#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>

using CubeCoordinates = HPM::dataType::Vec3D;
using CubeMesh = HPM::mesh::Mesh<CubeCoordinates, HPM::entity::Simplex>;

struct UnitCube
{

    const std::vector<CubeCoordinates> nodes{{0., 0., 0.}, {0., 0., 1.}, {1., 0., 0.}, {0., 1., 0.}, {1., 0., 1.}, {0., 1., 1.}, {1., 1., 0.}, {1., 1., 1.}};

    const std::vector<std::array<std::size_t, 4>> simplexes{{0, 6, 5, 3}, {0, 2, 4, 6}, {6, 4, 7, 5}, {0, 4, 1, 5}, {0, 4, 5, 6}};

    CubeMesh mesh{nodes, simplexes};

    static constexpr auto NumCells = 5;
    static constexpr auto NumFaces = 16;
    static constexpr auto NumEdges = 18;
    static constexpr auto NumNodes = 8;

    static constexpr auto MiddleCellIndex = 4;
};

#endif
