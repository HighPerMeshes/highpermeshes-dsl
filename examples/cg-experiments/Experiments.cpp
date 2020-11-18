#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include "ForwardEuler.hpp"
#include "ForwardEulerOCL.hpp"
#include "RungeKutta.hpp"
#include "RungeKuttaOCL.hpp"
#include "Volume.hpp"
#include "VolumeOCL.hpp"
#include "RungeKuttaStripped.hpp"
#include "memcopy.hpp"
#include "memcopyOCL.hpp"
#include "RungeKuttaVolume.hpp"
#include "RungeKuttaVolumeOCL.hpp"

#include <cmath>

using namespace HPM;

template <typename BufferA, typename BufferB>
void assign(BufferA &a, const BufferB &b)
{
#pragma omp parallel for
    for (size_t i = 0; i < a.GetSize(); ++i)
    {
        a[i] = b[i];
    }
}

template <typename BufferA, typename BufferB>
auto find_inequalities(BufferA &a, const BufferB &b)
{
    std::vector<size_t> differences;
    for (size_t i = 0; i < a.GetSize(); ++i)
    {
        if (a[i] != b[i])
            differences.push_back(i);
    }
    return differences;
}

template <typename BufferA, typename BufferB>
auto print_inequalities(const std::vector<size_t> &inequalities, const BufferA &a, const BufferB &b)
{
    for (const auto &i : inequalities)
    {
        std::cout << "\t\t\tindex: " << i << ", difference: " << a[i] - b[i] << "\n";
    }
}

int main(int argc, char **argv)
{

    const HPM::auxiliary::ConfigParser hpm_config_parser("config.cfg");
    const std::string hpm_ocl_platform_name = hpm_config_parser.GetValue<std::string>("oclPlatformName");
    const std::string hpm_ocl_device_name = hpm_config_parser.GetValue<std::string>("oclDeviceName");
    HPM::OpenCLHandler hpm_ocl(hpm_ocl_platform_name, hpm_ocl_device_name);

    auto analyze = [](auto seq_result, auto par_result, size_t iteration_mod) {
        double seq_time = static_cast<double>(seq_result);
        double par_time = static_cast<double>(par_result);

        std::cout << "\tSequential Time = " << seq_result << " ns, Avgerage = " << seq_time / iteration_mod << " ns \n"
                  << "\tParallel Time = " << par_time << " ns, Average = " << par_time / iteration_mod << " ns \n"
                  << "\tSpeedup: " << seq_time / par_time << "\n";
    };

    auto inequalities = [](const auto &buffersA, const auto &buffersB) {
        std::cout << "\tinequalities: {\n";
        HPM::auxiliary::ConstexprFor<std::tuple_size_v<std::decay_t<decltype(buffersA)>>>([&](auto index) {
            constexpr auto i = decltype(index)::value;
            std::cout << "\t\tBuffer " << i << ":\n\t\t{\n";
            auto ineqs = find_inequalities(std::get<i>(buffersA), std::get<i>(buffersB));
            print_inequalities(ineqs, std::get<i>(buffersA), std::get<i>(buffersB));
            std::cout << "\t\t}\n";
        });
        std::cout << "\t}\n";
    };

    auto assign_all = [](auto &buffersA, const auto &buffersB) {
        HPM::auxiliary::ConstexprFor<std::tuple_size_v<std::decay_t<decltype(buffersA)>>>([&](auto index) {
            constexpr auto i = decltype(index)::value;
            assign(std::get<i>(buffersA), std::get<i>(buffersB));
        });
    };

    drts::Runtime runtime{
        GetBuffer{}};

    HPM::drts::Runtime ocl_runtime{HPM::GetBuffer<HPM::OpenCLHandler::SVMAllocator>{}};

    size_t mesh_mod = 10;
    size_t iteration_mod = 1000;

    Grid<3> grid{{10 * mesh_mod, 10, 10}};
    const auto &mesh = grid.mesh;

    std::cout << "Tetrahedras: " << mesh.template GetNumEntities<3>() << "\nIterations: " << iteration_mod << "\n";

    using CoordinateType = dataType::Vec<double, 3>;

    {
        constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

        auto buffers = MakeTuple<2>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<double>(mesh, Dofs);
                fill_random(buffer);
                return buffer;
            });

        auto ocl_buffers = MakeTuple<2>(
            [&](auto) {
                return ocl_runtime.GetBuffer<double>(mesh, Dofs, hpm_ocl.GetSVMAllocator<double>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Forward Euler: {\n";
        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    {

        constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

        auto buffers = MakeTuple<6>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
                fill_ones(buffer);
                return buffer;
            });

        auto ocl_buffers = MakeTuple<6>(
            [&](auto) {
                return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<HPM::dataType::Vec<double, 3>>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Runge Kutta: {\n";

        analyze(
            RungeKutta(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers),
            iteration_mod);

        // inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    // {
    //     using CoordinateType = dataType::Vec<double, 3>;

    //     constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

    //     auto buffers = MakeTuple<4>(
    //         [&](auto) {
    //             auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
    //             fill_ones(buffer);
    //             return buffer;
    //         });

    //     auto ocl_buffers = MakeTuple<4>(
    //         [&](auto) {
    //             return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<CoordinateType>());
    //         });

    //     assign_all(ocl_buffers, buffers);

    //     std::cout << "Volume: {\n";

    //     VolumeOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers),


    //     // analyze(
    //     //     Volume(mesh, iteration_mod, buffers),
    //     //     VolumeOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers),
    //     //     iteration_mod);

    //     inequalities(ocl_buffers, buffers);
    //     std::cout << "}\n";
    // }
}