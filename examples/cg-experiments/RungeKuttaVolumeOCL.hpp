#ifndef RUNGEKUTTAVOLUMEOCL_HPP
#define RUNGEKUTTAVOLUMEOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Mesh>
auto RungeKuttaVolumeOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl)
{

    std::fstream hpm_kernel_stream{"RungeKuttaVolume.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"rk_function_17", "function_30"});

    //The runtime determines the configuration of HighPerMeshes.
    //The GetBuffer class determines that we use a normal buffer to allocate space
    HPM::drts::Runtime hpm{HPM::GetBuffer<HPM::OpenCLHandler::SVMAllocator>{}};

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
            auto buffer = hpm.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<HPM::dataType::Vec<double, 3>>());
            fill_random(buffer);
            return buffer;
        });

    using namespace HPM::dataType;

    auto rk_kernel = HPM::ForEachEntity(
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
        HPM::internal::OpenMP_ForEachEntity<3>{});

    auto vol_kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            Read(Cell(std::get<0>(buffers))),
            Read(Cell(std::get<1>(buffers))),
            Cell(std::get<2>(buffers)),
            Cell(std::get<3>(buffers))),
        [&](const auto &element, const auto &, auto &lvs) {
            const Mat3D &D = element.GetGeometry().GetInverseJacobian() * 2.0;

            HPM::ForEach(numVolNodes, [&](const std::size_t n) {
                Mat3D derivative_E;
                Mat3D derivative_H; //!< derivative of fields w.r.t reference coordinates

                const auto &fieldH = std::get<0>(lvs);
                const auto &fieldE = std::get<1>(lvs);

                HPM::ForEach(numVolNodes, [&](const std::size_t m) {
                    derivative_H += DyadicProduct(derivative[n][m], fieldH[m]);
                    derivative_E += DyadicProduct(derivative[n][m], fieldE[m]);
                });

                auto &rhsH = std::get<2>(lvs);
                auto &rhsE = std::get<3>(lvs);

                rhsH[n] += -Curl(D, derivative_E); //!< first half of right-hand-side of fields
                rhsE[n] += Curl(D, derivative_H);
            });
        }
        );

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {
                       auto hpm_kernel_0 = rk_kernel;
                       auto buffers_0 = GetBuffers(hpm_kernel_0);
                       auto offsets_0 = GetOffsets(hpm_kernel_0);
                       auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "rk_function_17", std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), 1}.with(buffers_0).with(offsets_0);
                       
                       auto hpm_kernel_1 = vol_kernel;
                       auto buffers_1 = GetBuffers(hpm_kernel_1);
                       auto offsets_1 = GetOffsets(hpm_kernel_1);
                       auto hpm_ocl_kernel_1 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_30", std::tuple<unsigned long>{0}, hpm_kernel_1.entity_range.GetSize(), 1}.with(buffers_1).with(offsets_1).with(std::tuple{GetInverseJacobian(hpm_kernel_1.entity_range)});
                                             
                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{iteration_mod}, hpm_ocl_kernel_0 , hpm_ocl_kernel_1 );

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

#endif /* RUNGEKUTTAOCL_HPP */
