#ifndef VOLUME_HPP
#define VOLUME_HPP

#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Range, typename Buffers>
auto VolumeKernel(Range &range, Buffers &buffers)
{
    using namespace HPM::dataType;

    return HPM::ForEachEntity(
        range,
        std::tuple(
            Read(Cell(std::get<0>(buffers))),
            Read(Cell(std::get<1>(buffers))),
            Cell(std::get<2>(buffers)),
            Cell(std::get<3>(buffers))),
        [&](const auto &element, const auto &, auto &lvs) {
            const Mat3D &D = element.GetGeometry().GetInverseJacobian() * 2.0;

            const auto &fieldH = std::get<0>(lvs);
            const auto &fieldE = std::get<1>(lvs);

            auto &rhsH = std::get<2>(lvs);
            auto &rhsE = std::get<3>(lvs);


            HPM::ForEach(numVolNodes, [&](const std::size_t n) {
                Mat3D derivative_E;
                Mat3D derivative_H; //!< derivative of fields w.r.t reference coordinates

                
                HPM::ForEach(numVolNodes, [&](const std::size_t m) {
                    derivative_H += DyadicProduct(derivative[n][m], fieldH[m]);
                    derivative_E += DyadicProduct(derivative[n][m], fieldE[m]);

                    rhsH[n] += derivative_H[0];

                });

                rhsH[n] += D[0];
                rhsH[n] += -Curl(D, derivative_E); //!< first half of right-hand-side of fields
                rhsE[n] += Curl(D, derivative_H);
            });
        },
        HPM::internal::OpenMP_ForEachEntity<3>{});
}

template <typename Mesh, typename Buffers>
auto Volume(const Mesh &mesh, size_t iteration_mod, Buffers &buffers)
{
    static constexpr auto CellDimension = 3;

    const auto AllCells{
        mesh.template GetEntityRange<CellDimension>()};

    SequentialDispatcher dispatcher;

    using namespace HPM::dataType;

    auto kernel = VolumeKernel(AllCells, buffers);

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   dispatcher.Execute(
                       HPM::iterator::Range{iteration_mod},
                       kernel);
               })
        .count();
}

#endif /* VOLUME_HPP */
