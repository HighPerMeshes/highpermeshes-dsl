#ifndef FORWARDEULEROCL_HPP
#define FORWARDEULEROCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Mesh>
auto ForwardEulerOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl)
{

    std::fstream hpm_kernel_stream{"ForwardEuler.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_1"});

    //The runtime determines the configuration of HighPerMeshes.
    //The GetBuffer class determines that we use a normal buffer to allocate space
    HPM::drts::Runtime hpm{HPM::GetBuffer<HPM::OpenCLHandler::SVMAllocator>{}};

    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other.
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    const auto AllCells{
        mesh.template GetEntityRange<0>()};

    constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

    auto buffers = MakeTuple<2>(
        [&](auto) {
            return hpm.GetBuffer<double>(mesh, Dofs, hpm_ocl.GetSVMAllocator<double>());
        });

    auto kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            ReadWrite(Node(std::get<0>(buffers))),
            Read(Node(std::get<1>(buffers)))),
        [&](const auto &, const auto &, auto &lvs) {
            auto &u = dof::GetDofs<0>(std::get<0>(lvs));
            const auto &u_d = dof::GetDofs<0>(std::get<1>(lvs));

            auto tau = 0.2;

            u[0] += tau * u_d[0];
        });

    auto data_size = kernel.entity_range.GetSize() * 3 * sizeof(double);

    return HPM::auxiliary::MeasureTime(
                         [&]() {
                             {
                                 auto hpm_kernel_0 = kernel;
                                 auto mesh_info_0 = MakeMeshInfo(hpm_ocl, hpm_kernel_0.entity_range.GetMesh());
                                 auto buffers_0 = GetBuffers(hpm_kernel_0);
                                 auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_1", std::tuple{mesh_info_0, static_cast<int>(mesh_info_0.size()), size_t { 0 } }, hpm_kernel_0.entity_range.GetSize()}.with(buffers_0);
                                 HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range { iteration_mod }, hpm_ocl_kernel_0);
                                 hpm_ocl.GetDefaultQueue().finish();
                             };
                         })
                         .count();
}

#endif /* FORWARDEULEROCL_HPP */
