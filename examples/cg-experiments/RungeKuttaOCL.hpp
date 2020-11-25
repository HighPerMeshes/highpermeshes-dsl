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
auto RungeKuttaOCL(const Mesh &mesh, size_t iteration_mod, HPM::OpenCLHandler &hpm_ocl, Buffers& buffers, size_t work_group_size, std::string file_name, std::string kernel_name)
{

    std::fstream hpm_kernel_stream{ file_name};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, { kernel_name } );

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

    using namespace HPM::dataType;

    auto kernel = RKKernel(AllCells, buffers);

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {

                       auto hpm_kernel_0 = kernel;
                       auto buffers_0 = GetBuffers(hpm_kernel_0);
                       auto offsets_0 = GetOffsets(hpm_kernel_0);

                       auto hpm_ocl_kernel_0 = HPM::OpenCLKernelEnqueuer{hpm_ocl, kernel_name, std::tuple<unsigned long>{0}, hpm_kernel_0.entity_range.GetSize(), work_group_size}.with(buffers_0).with(offsets_0);
                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{iteration_mod}, hpm_ocl_kernel_0);

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

#endif /* RUNGEKUTTAOCL_HPP */
