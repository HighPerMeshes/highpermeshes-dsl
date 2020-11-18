#ifndef MEMCOPY_HPP
#define MEMCOPY_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename T> struct DEBUG;

template<typename Mesh>
auto Memcopy(const Mesh& mesh, size_t iteration_mod)
{
    //The runtime determines the configuration of HighPerMeshes.
    //The GetBuffer class determines that we use a normal buffer to allocate space
    drts::Runtime hpm{
        GetBuffer{}};

    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other.
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

    auto buffers = MakeTuple<2>(
        [&](auto) {
            return std::vector<double>(mesh.template GetNumEntities<0>() * Dofs.At<0>());
        });

    return HPM::auxiliary::MeasureTime(
        [&]() {

            for(size_t step = 0; step < iteration_mod; ++ step) {

                #pragma omp parallel for
                for(size_t i = 0; i < std::get<0>(buffers).size(); ++i) {
                    std::get<0>(buffers)[i] = std::get<1>(buffers)[i];
                }

            }

        }
    ).count();
}
#endif /* FORWARDEULER_HPP */
