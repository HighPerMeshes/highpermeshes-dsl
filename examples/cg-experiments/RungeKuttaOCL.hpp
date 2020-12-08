#ifndef RUNGEKUTTAOCL_HPP
#define RUNGEKUTTAOCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include "RungeKutta.hpp"

using namespace HPM;

template <typename Mesh, typename Buffers>
auto RungeKuttaOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers &buffers, size_t work_group_size, std::string file_name, std::string kernel_name)
{

    load_kernel(hpm_ocl, file_name, kernel_name);
    const auto AllCells{
        mesh.template GetEntityRange<3>()};

    auto kernel = RKKernel(AllCells, buffers);

    return MeasureOCL(hpm_ocl, kernel_name, kernel, iteration_mod, work_group_size);

}

template <typename Mesh, typename Buffers>
auto RungeKuttaOCLKernelTime(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers &buffers, size_t work_group_size, std::string file_name, std::string kernel_name)
{

    std::fstream hpm_kernel_stream{file_name};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {kernel_name});

    HPM::drts::Runtime hpm{HPM::GetBuffer<HPM::OpenCLHandler::SVMAllocator>{}};

    const auto AllCells{
        mesh.template GetEntityRange<3>()};

    using namespace HPM::dataType;

    auto kernel = RKKernel(AllCells, buffers);
    auto hpm_kernel_0 = kernel;
    auto buffers_0 = GetBuffers(hpm_kernel_0);
    auto offsets_0 = GetOffsets(hpm_kernel_0);

    auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, kernel_name, std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), work_group_size}.with(buffers_0).with(offsets_0);

    return HPM::OpenCLDispatcher{}.MeasureDispatch(HPM::iterator::Range{iteration_mod}, hpm_ocl_kernel_0);
}

#endif /* RUNGEKUTTAOCL_HPP */
