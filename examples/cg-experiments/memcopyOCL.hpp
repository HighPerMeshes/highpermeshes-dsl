#ifndef MEMCOPYOCL_HPP
#define MEMCOPYOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Mesh>
auto MemcopyOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl)
{
    std::fstream hpm_kernel_stream{"memcopy.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());
    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_0"});

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
            std::vector<double, std::decay_t<decltype(hpm_ocl.GetSVMAllocator<double>())>> vec(hpm_ocl.GetSVMAllocator<double>());
            vec.resize(mesh.template GetNumEntities<0>() * Dofs.At<0>());
            return vec;
        });

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {

                       auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_0", std::tuple{}, std::get<0>(buffers).size(), 1}.with(buffers);
                       hpm_ocl_kernel_0.unmap();
                       
                       for(size_t step = 0; step < iteration_mod; ++step) {
                           hpm_ocl_kernel_0.enqueue();
                       }
                       hpm_ocl_kernel_0.map();

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

#endif /* FORWARDEULEROCL_HPP */
