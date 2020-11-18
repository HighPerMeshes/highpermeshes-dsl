#ifndef RUNGEKUTTA_HPP
#define RUNGEKUTTA_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Range, typename Buffers>
auto RKKernel(Range &range, Buffers &buffers)
{
    using namespace HPM::dataType;

    return HPM::ForEachEntity(
        range,
        std::tuple(
            Write(Cell(std::get<0>(buffers))),
            Write(Cell(std::get<1>(buffers))),
            Cell(std::get<2>(buffers)),
            Cell(std::get<3>(buffers)),
            Cell(std::get<4>(buffers)),
            Cell(std::get<5>(buffers))),
        [&](const auto &, const auto &iter, auto lvs) {
            const auto &RKstage = rk4[iter % 5];

            auto &fieldH = std::get<0>(lvs);
            auto &fieldE = std::get<1>(lvs);
            auto &rhsH = std::get<2>(lvs);
            auto &rhsE = std::get<3>(lvs);
            auto &resH = std::get<4>(lvs);
            auto &resE = std::get<5>(lvs);

            HPM::ForEach(numVolNodes, [&](const std::size_t n) {
                resH[n] = RKstage[0] * resH[n] + /* timeStep * */ rhsH[n]; //!< residual fields
                resE[n] = RKstage[0] * resE[n] + /* timeStep * */ rhsE[n];
                fieldH[n] += RKstage[1] * resH[n]; //!< updated fields
                fieldE[n] += RKstage[1] * resE[n];
                assign_to_entries(rhsH[n], 0.0);
                assign_to_entries(rhsE[n], 0.0);
            });
        },
        HPM::internal::OpenMP_ForEachEntity<3> {}
        );
}

template<typename Mesh, typename Buffers>
auto RungeKutta(const Mesh& mesh, size_t iteration_mod, Buffers& buffers)
{
    static constexpr auto CellDimension = 3;

    const auto AllCells{
        mesh.template GetEntityRange<CellDimension>()};


    SequentialDispatcher dispatcher;

    using namespace HPM::dataType;

    auto kernel = RKKernel(AllCells, buffers);

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
