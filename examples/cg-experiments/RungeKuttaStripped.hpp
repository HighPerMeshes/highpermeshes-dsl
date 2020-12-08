#ifndef RUNGEKUTTASTRIPPED_HPP
#define RUNGEKUTTASTRIPPED_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename Mesh, typename Buffers>
auto RungeKuttaStripped(const Mesh& mesh, size_t iteration_mod, Buffers &buffers)
{
    //The runtime determines the configuration of HighPerMeshes.
    //The GetBuffer class determines that we use a normal buffer to allocate space
    drts::Runtime hpm{
        GetBuffer{}};

    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other.
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    // The CoordinateType tells us which data type and which dimensionality to use for a given mesh.
    using CoordinateType = dataType::Vec<double, 3>;

    static constexpr auto CellDimension = 3;

    const auto AllCells{
        mesh.template GetEntityRange<CellDimension>()};

    auto measured = HPM::auxiliary::MeasureTime(
        [&]() {
            
            for(size_t iter = 0; iter < iteration_mod; ++iter) {

                #pragma omp parallel for
                for(size_t cell = 0; cell < mesh.template GetNumEntities<3>(); ++cell) {
                    const auto &RKstage = rk4[iter % 5];

                    auto &fieldH = std::get<0>(buffers);
                    auto &fieldE = std::get<1>(buffers);
                    auto &rhsH = std::get<2>(buffers);
                    auto &rhsE = std::get<3>(buffers);
                    auto &resH = std::get<4>(buffers);
                    auto &resE = std::get<5>(buffers);

                    for(size_t n_ = 0; n_ < numVolNodes; ++n_) {

                        size_t n = n_ + numVolNodes * cell;

                        resH[n] = RKstage[0] * resH[n] + /* timeStep * */ rhsH[n]; //!< residual fields
                        resE[n] = RKstage[0] * resE[n] + /* timeStep * */ rhsE[n];
                        fieldH[n] += RKstage[1] * resH[n]; //!< updated fields
                        fieldE[n] += RKstage[1] * resE[n];
                        assign_to_entries(rhsH[n], 0.0);
                        assign_to_entries(rhsE[n], 0.0);

                    }
                }
            }
        }
    ).count();

    return measured;


}
#endif /* RUNGEKUTTA_HPP */
