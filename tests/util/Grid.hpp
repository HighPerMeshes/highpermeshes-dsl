// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef TEST_GRID_HPP
#define TEST_GRID_HPP

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include <HighPerMeshes/common/Vec.hpp>
#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>

#include <ostream>

template <std::size_t Dimension>
struct Grid;

template <>
struct Grid<3>
{
    using CoordinateT = ::HPM::dataType::Vec<double, 3>;
    using GridMesh = ::HPM::mesh::Mesh<CoordinateT, ::HPM::entity::Simplex>;

    Grid(const std::array<std::size_t, 3> &extent) : 
        nodes{
            [&extent]() {
                std::vector<CoordinateT> nodes;
                const std::size_t num_nodes = std::accumulate(extent.begin(), extent.end(), 1, [](const auto &a, const auto &b) { return a * b; });
                nodes.reserve(num_nodes);

                for (std::size_t z = 0; z < extent[2]; ++z)
                {
                    for (std::size_t y = 0; y < extent[1]; ++y)
                    {
                        for (std::size_t x = 0; x < extent[0]; ++x)
                        {
                            nodes.push_back({x, y, z});
                        }
                    }
                }
                return nodes;
            }()},
        simplices{[&extent]() {
            std::vector<std::array<std::size_t, 4>> simplices;
            const std::size_t simplices_per_unit_cell = 5;
            const std::size_t num_simplices = std::accumulate(extent.begin(), extent.end(), simplices_per_unit_cell, [](const auto &a, const auto &b) { return a * (b - 1); });
            simplices.reserve(num_simplices);

            for (std::size_t z = 0; z < (extent[2] - 1); ++z)
            {
                for (std::size_t y = 0; y < (extent[1] - 1); ++y)
                {
                    for (std::size_t x = 0; x < (extent[0] - 1); ++x)
                    {
                        const std::size_t origin = (z * extent[1] + y) * extent[0] + x;
                        const std::size_t p_1 = origin;
                        const std::size_t p_2 = origin + 1;
                        const std::size_t p_3 = origin + extent[0];
                        const std::size_t p_4 = origin + extent[0] + 1;
                        const std::size_t p_5 = origin + extent[0] * extent[1];
                        const std::size_t p_6 = origin + extent[0] * extent[1] + 1;
                        const std::size_t p_7 = origin + extent[0] * extent[1] + extent[0];
                        const std::size_t p_8 = origin + extent[0] * extent[1] + extent[0] + 1;

                        simplices.push_back({p_1, p_3, p_4, p_7});
                        simplices.push_back({p_1, p_2, p_4, p_6});
                        simplices.push_back({p_4, p_6, p_7, p_8});
                        simplices.push_back({p_1, p_5, p_6, p_7});
                        simplices.push_back({p_1, p_4, p_6, p_7});
                    }
                }
            }
            return simplices;
        }()},
        mesh{nodes, simplices}

    {
        assert(extent[0] > 1);
        assert(extent[1] > 1);
        assert(extent[2] > 1);
    }

    std::vector<CoordinateT> nodes;
    std::vector<std::array<std::size_t, 4>> simplices;
    GridMesh mesh;
};

template <>
struct Grid<2>
{
    using CoordinateT = ::HPM::dataType::Vec<double, 2>;
    using GridMesh = ::HPM::mesh::Mesh<CoordinateT, ::HPM::entity::Simplex>;

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
    Grid(std::size_t horizontal_count, std::size_t vertical_count) : 
        nodes{
            [horizontal_count, vertical_count]() {
                std::vector<CoordinateT> nodes;
                for (std::size_t vertical = 0; vertical <= vertical_count; ++vertical)
                {
                    for (std::size_t horizontal = 0; horizontal <= horizontal_count; ++horizontal)
                    {
                        nodes.emplace_back(CoordinateT{static_cast<double>(horizontal), static_cast<double>(vertical)});
                    }
                }
                return nodes;
            }()},
        simplices{[horizontal_count, vertical_count]() {
            std::vector<std::array<std::size_t, 3>> simplices;

            for (std::size_t vertical = 0; vertical < vertical_count; ++vertical)
            {
                for (std::size_t horizontal = 0; horizontal < horizontal_count; ++horizontal)
                {

                    auto up = horizontal_count + 1;
                    auto pos = up * vertical + horizontal;

                    simplices.emplace_back(std::array{pos, pos + 1, pos + up});
                    simplices.emplace_back(std::array{pos + 1, pos + up, pos + up + 1});
                }
            }
            return simplices;
        }()},
        mesh{nodes, simplices}

    {
    }

    Grid(const std::array<std::size_t, 2> &extent) : Grid(extent[0], extent[1]) {}

    std::vector<CoordinateT> nodes;
    std::vector<std::array<std::size_t, 3>> simplices;
    GridMesh mesh;

    friend auto &operator<<(std::ostream &os, Grid g)
    {
        std::cout << "nodes:" << std::endl;
        for (auto node : g.nodes)
        {
            std::cout << node << std::endl;
        }
        std::cout << "simplices:" << std::endl;
        for (auto entity : g.simplices)
        {
            std::cout << "{" << entity[0] << ", " << entity[1] << ", " << entity[2] << "}" << std::endl;
        }
        return os;
    }
};

#endif
