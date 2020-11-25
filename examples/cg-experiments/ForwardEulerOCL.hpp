#ifndef FORWARDEULEROCL_HPP
#define FORWARDEULEROCL_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

template <typename Mesh, typename Buffers>
auto ForwardEulerOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers& buffers, size_t work_group_size)
{
    std::fstream hpm_kernel_stream{"ForwardEuler.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());
    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"function_4"});

    const auto AllCells{
        mesh.template GetEntityRange<0>()};

    SequentialDispatcher dispatcher;

    auto kernel = HPM::ForEachEntity(
        AllCells,
        std::tuple(
            ReadWrite(Node(std::get<0>(buffers))),
            Read(Node(std::get<1>(buffers)))),
        [&](const auto &, const auto &, auto &lvs) {
            auto &u = std::get<0>(lvs);
            const auto &u_d = std::get<1>(lvs);

            auto tau = 0.2;

            u[0] += tau * u_d[0];
        },
        HPM::internal::OpenMP_ForEachEntity<0>{});

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {
                       auto hpm_kernel_0 = kernel;
                       auto buffers_0 = GetBuffers(hpm_kernel_0);
                       auto offsets_0 = GetOffsets(hpm_kernel_0);
                       auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, "function_4", std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), work_group_size}.with(buffers_0).with(offsets_0);
                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{iteration_mod}, hpm_ocl_kernel_0);

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

#endif /* FORWARDEULEROCL_HPP */
