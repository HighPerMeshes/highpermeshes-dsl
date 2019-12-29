// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef TEST_GRID_HPP
#define TEST_GRID_HPP

#include <HighPerMeshes/common/Vec.hpp>
#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>

using GridCoordinates = HPM::dataType::Vec<double, 2>;
using GridMesh = HPM::mesh::Mesh<GridCoordinates, HPM::entity::Simplex>;

#include <ostream>

struct Grid
{
    //! Generates a grid mesh in 2D with a number of squares built from triangles equal to horizontal_count * vertical_count
    //!
    //! For a (1, 1) grid the topology is:
    //!
    //  2 -- 3
    //! | \  |
    //! |  \ |
    //! 0 -- 1
    //!
    //! And the coordinates are
    //!
    //! (0,1) -- (1,1)
    //!   |   \    |
    //!   |    \   |
    //! (0,0) -- (1,0)
    Grid(std::size_t horizontal_count, std::size_t vertical_count)
    {
        for (std::size_t vertical = 0; vertical <= vertical_count; ++vertical)
        {
            for (std::size_t horizontal = 0; horizontal <= horizontal_count; ++horizontal)
            {
                nodes.emplace_back(GridCoordinates{static_cast<double>(horizontal), static_cast<double>(vertical)});
            }
        }
        for (std::size_t vertical = 0; vertical < vertical_count; ++vertical)
        {
            for (std::size_t horizontal = 0; horizontal < horizontal_count; ++horizontal)
            {

                auto up = horizontal_count + 1;
                auto pos = up * vertical + horizontal;

                simplexes.emplace_back(std::array{pos, pos + 1, pos + up});
                simplexes.emplace_back(std::array{pos + 1, pos + up, pos + up + 1});
            }
        }
    }

    std::vector<GridCoordinates> nodes;

    std::vector<std::array<std::size_t, 3>> simplexes;

    GridMesh mesh{nodes, simplexes};

    friend auto& operator<<(std::ostream& os, Grid g)
    {
        std::cout << "nodes:" << std::endl;
        for (auto node : g.nodes)
        {
            std::cout << node << std::endl;
        }
        std::cout << "simplexes:" << std::endl;
        for (auto entity : g.simplexes)
        {
            std::cout << "{" << entity[0] << ", " << entity[1] << ", " << entity[2] << "}" << std::endl;
        }
        return os;
    }
};

#endif
