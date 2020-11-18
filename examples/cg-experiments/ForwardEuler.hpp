#ifndef FORWARDEULER_HPP
#define FORWARDEULER_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename T> struct DEBUG;

template<typename Mesh, typename Buffers>
auto ForwardEuler(const Mesh& mesh, size_t iteration_mod, Buffers& buffers)
{
    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other.
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    const auto AllCells{
        mesh.template GetEntityRange<0>()};

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
        },
         HPM::internal::OpenMP_ForEachEntity<0> {}
        );

    return HPM::auxiliary::MeasureTime(
        [&]() {
            dispatcher.Execute(
                HPM::iterator::Range { iteration_mod },
                kernel
            );
        }
    ).count();
}
#endif /* FORWARDEULER_HPP */
