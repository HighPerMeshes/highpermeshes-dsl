#ifndef VOLUMEOCL_HPP
#define VOLUMEOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename Mesh>
auto VolumeOCL(const Mesh& mesh, size_t iteration_mod, HPM::OpenCLHandler& hpm_ocl)
{

    std::fstream hpm_kernel_stream{"Volume.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());


    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_30"});

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

    auto buffers = MakeTuple<4>(
        [&](auto) {
            return hpm.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<HPM::dataType::Vec<double, 3>>());
        });

    using namespace HPM::dataType;

    auto kernel = HPM::ForEachEntity(
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

                const auto &fieldH = dof::GetDofs<3>(std::get<0>(lvs));
                const auto &fieldE = dof::GetDofs<3>(std::get<1>(lvs));

                HPM::ForEach(numVolNodes, [&](const std::size_t m) {
                    derivative_H += DyadicProduct(derivative[n][m], fieldH[m]);
                    derivative_E += DyadicProduct(derivative[n][m], fieldE[m]);
                });

                auto &rhsH = dof::GetDofs<3>(std::get<2>(lvs));
                auto &rhsE = dof::GetDofs<3>(std::get<3>(lvs));

                rhsH[n] += -Curl(D, derivative_E); //!< first half of right-hand-side of fields
                rhsE[n] += Curl(D, derivative_H);
            });
        });

    return HPM::auxiliary::MeasureTime(
        [&]() {
            {
                auto hpm_kernel_0 = kernel;
                auto mesh_info_0 = MakeMeshInfo(hpm_ocl, hpm_kernel_0.entity_range.GetMesh());
                auto buffers_0 = GetBuffers(hpm_kernel_0);
                auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_30", std::tuple{mesh_info_0, static_cast<int>(mesh_info_0.size()), size_t { 0 } }, hpm_kernel_0.entity_range.GetSize()}.with(buffers_0).with(std::tuple{GetInverseJacobian(hpm_kernel_0.entity_range)});
                HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range { iteration_mod }, hpm_ocl_kernel_0);
                hpm_ocl.GetDefaultQueue().finish();
            };
        }).count();

}

#endif /* VOLUMEOCL_HPP */
