#ifndef FORWARDEULER_HPP
#define FORWARDEULER_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename T> struct DEBUG;

int main()
{
    //The runtime determines the configuration of HighPerMeshes.
    //The GetBuffer class determines that we use a normal buffer to allocate space
    HPM::drts::Runtime hpm{ HPM::GetBuffer{} };

    Grid<3> grid{{10, 10, 10}};
    const auto& mesh = grid.mesh;

    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other.
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    const auto AllCells{
        mesh.template GetEntityRange<0>()};

    constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

    auto buffers = MakeTuple<2>(
        [&](auto) {
            return hpm.GetBuffer<CoordinateType>(mesh, Dofs);
        });

    SequentialDispatcher dispatcher;

    auto kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            ReadWrite(Node(std::get<0>(buffers))),
            Read(Node(std::get<1>(buffers)))),
        [&](const auto &, const auto &, auto &lvs) {
            
            auto &u = std::get<0>(lvs);
            const auto &u_d = std::get<1>(lvs);
            
            auto tau = 0.2;

            u[0] += tau * u_d[0];
        }
        , HPM::internal::OpenMP_ForEachEntity<0>{}
        );

    return HPM::auxiliary::MeasureTime(
        [&]() {
            dispatcher.Execute(
                HPM::iterator::Range { size_t { 1 } },
                kernel
            );
        }
    ).count();
}
#endif /* FORWARDEULER_HPP */

