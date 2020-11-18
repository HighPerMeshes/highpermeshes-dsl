#ifndef VOLUMEOCL_HPP
#define VOLUMEOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include "Volume.hpp"

using namespace HPM;

template <typename Mesh, typename Buffers>
auto VolumeOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers& buffers)
{

    std::fstream hpm_kernel_stream{"Volume.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_2"});

    const auto AllCells{
        mesh.template GetEntityRange<3>()};

    using namespace HPM::dataType;

    auto kernel = VolumeKernel(AllCells, buffers);

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {
                       auto hpm_kernel_0 = kernel;
                       auto buffers_0 = GetBuffers(hpm_kernel_0);
                       auto offsets_0 = GetOffsets(hpm_kernel_0);
                       auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_2", std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), 1}.with(buffers_0).with(offsets_0); //.with(std::tuple{GetInverseJacobian(hpm_kernel_0.entity_range)});
                       
                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{size_t{iteration_mod}}, hpm_ocl_kernel_0);

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

#endif /* VOLUMEOCL_HPP */
