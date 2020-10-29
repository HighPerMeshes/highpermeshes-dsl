#ifndef RUNGEKUTTA_HPP
#define RUNGEKUTTA_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename Mesh>
auto RungeKutta(const Mesh& mesh, size_t iteration_mod)
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

    constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

    auto buffers = MakeTuple<6>(
        [&](auto) {
            return hpm.GetBuffer<CoordinateType>(mesh, Dofs);
        });

    SequentialDispatcher dispatcher;

    using namespace HPM::dataType;

    auto kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            Write(Cell(std::get<0>(buffers))),
            Write(Cell(std::get<1>(buffers))),
            Cell(std::get<2>(buffers)),
            Cell(std::get<3>(buffers)),
            Cell(std::get<4>(buffers)),
            Cell(std::get<5>(buffers))),
        [&](const auto &, const auto &iter, auto lvs) {
            const auto &RKstage = rk4[iter % 5];

            auto &fieldH = dof::GetDofs<CellDimension>(std::get<0>(lvs));
            auto &fieldE = dof::GetDofs<CellDimension>(std::get<1>(lvs));
            auto &rhsH = dof::GetDofs<CellDimension>(std::get<2>(lvs));
            auto &rhsE = dof::GetDofs<CellDimension>(std::get<3>(lvs));
            auto &resH = dof::GetDofs<CellDimension>(std::get<4>(lvs));
            auto &resE = dof::GetDofs<CellDimension>(std::get<5>(lvs));

            HPM::ForEach(numVolNodes, [&](const std::size_t n) {
                resH[n] = RKstage[0] * resH[n] + /* timeStep * */ rhsH[n]; //!< residual fields
                resE[n] = RKstage[0] * resE[n] + /* timeStep * */ rhsE[n];
                fieldH[n] += RKstage[1] * resH[n]; //!< updated fields
                fieldE[n] += RKstage[1] * resE[n];
                assign_to_entries(rhsH[n], 0.0);
                assign_to_entries(rhsE[n], 0.0);
            });
        }
        ,HPM::internal::OpenMP_ForEachEntity<3>{}
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
#endif /* RUNGEKUTTA_HPP */
