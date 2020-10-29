#ifndef RUNGEKUTTAOCL_HPP
#define RUNGEKUTTAOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template<typename Mesh>
auto RungeKuttaOCL(const Mesh& mesh, size_t iteration_mod, HPM::OpenCLHandler& hpm_ocl)
{

    std::fstream hpm_kernel_stream{"RungeKutta.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_17"});

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
            return hpm.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<HPM::dataType::Vec<double, 3>>());
        });

    using namespace HPM::dataType;

    auto kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            Cell(std::get<0>(buffers)),
            Cell(std::get<1>(buffers)),
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
        });

    auto data_size = kernel.entity_range.GetSize() * numVolNodes * 4 * 12 * sizeof(double);

    return std::pair { HPM::auxiliary::MeasureTime(
        [&]() {
            {
                auto hpm_kernel_0 = kernel;
                auto mesh_info_0 = MakeMeshInfo(hpm_ocl, hpm_kernel_0.entity_range.GetMesh());
                auto buffers_0 = GetBuffers(hpm_kernel_0);
                auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_17", std::tuple{mesh_info_0, static_cast<int>(mesh_info_0.size()), size_t { 0 } }, hpm_kernel_0.entity_range.GetSize()}.with(buffers_0);
                HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range { iteration_mod }, hpm_ocl_kernel_0);
                hpm_ocl.GetDefaultQueue().finish();
            };
        }).count(),

        data_size };
}

#endif /* RUNGEKUTTAOCL_HPP */
