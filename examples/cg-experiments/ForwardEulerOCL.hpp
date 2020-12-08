#ifndef FORWARDEULEROCL_HPP
#define FORWARDEULEROCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Mesh, typename Buffers>
auto ForwardEulerOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers &buffers, size_t work_group_size, std::string file_name, std::string kernel_name)
{
    load_kernel(hpm_ocl, file_name, kernel_name);
    auto range = mesh.template GetEntityRange<0>();

    auto kernel = ForwardEulerKernel(range, buffers);

    return MeasureOCL(hpm_ocl, kernel_name, kernel, iteration_mod, work_group_size);
}

template <typename Mesh, typename Buffers>
auto ForwardEulerOCLKernelTime(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers &buffers, size_t work_group_size, std::string file_name, std::string kernel_name)
{
    std::fstream hpm_kernel_stream{file_name};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());
    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {kernel_name});

    auto range = mesh.template GetEntityRange<0>();

    auto kernel = ForwardEulerKernel(range, buffers);

    auto hpm_kernel_0 = kernel;
    auto buffers_0 = GetBuffers(hpm_kernel_0);
    auto offsets_0 = GetOffsets(hpm_kernel_0);
    auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, kernel_name, std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), work_group_size}.with(buffers_0).with(offsets_0);

    return HPM::OpenCLDispatcher{}.MeasureDispatch(HPM::iterator::Range{iteration_mod}, hpm_ocl_kernel_0);
}

#endif /* FORWARDEULEROCL_HPP */
