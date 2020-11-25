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
#include "id_assign.hpp"

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

template <typename Buffer>
void print(Buffer &buffer)
{
    std::cout << "{\n\t";
    for (const auto &v : buffer)
    {
        std::cout << v << " ";
    }
    std::cout << "\n}\n";
}

template <typename T>
auto compare_epsilon(T lhs, T rhs, T epsilon)
{
    return std::abs(lhs - rhs) < epsilon;
}

template <typename T>
auto compare_epsilon(HPM::dataType::Vec<T, 3> lhs, HPM::dataType::Vec<T, 3> rhs, double epsilon)
{
    auto res = lhs - rhs;

    return std::abs(res[0]) < epsilon && std::abs(res[1]) < epsilon && std::abs(res[2]) < epsilon;
}

template <typename T>
auto max_error(T current, T rhs)
{
    return std::max(current, std::abs(rhs));
}

template <typename T>
auto max_error(HPM::dataType::Vec<T, 3> current, HPM::dataType::Vec<T, 3> rhs)
{
    return HPM::dataType::Vec<T, 3>{
        std::max(current[0], std::abs(rhs[0])),
        std::max(current[1], std::abs(rhs[1])),
        std::max(current[2], std::abs(rhs[2]))};
}

template <typename BufferA, typename BufferB>
auto find_inequalities(const BufferA &a, const BufferB &b, double epsilon)
{

    struct Result
    {
        std::vector<size_t> differences;
        typename BufferA::ValueT max_error{};
    } result;

    for (size_t i = 0; i < a.GetSize(); ++i)
    {
        if (!compare_epsilon(a[i], b[i], epsilon))
        {
            result.differences.push_back(i);
        }

        result.max_error = max_error(result.max_error, a[i] - b[i]);
    }

    return result;
}

template <typename BufferA, typename BufferB>
auto print_inequalities(const std::vector<size_t> &inequalities, const BufferA &a, const BufferB &b)
{
    for (const auto &i : inequalities)
    {
        std::cout << "\t\t\tindex: " << i << ", a[i]: " << a[i] << ", b[i]:" << b[i] << ", difference: " << a[i] - b[i] << "\n";
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
            auto ineqs = find_inequalities(std::get<i>(buffersA), std::get<i>(buffersB), 1.0E-12);
            print_inequalities(ineqs.differences, std::get<i>(buffersA), std::get<i>(buffersB));
            std::cout << "max error: " << ineqs.max_error << "\n";
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

    size_t mesh_mod = 1; // argc > 1 ? std::stoi(argv[1]) : 1;
    size_t iteration_mod = 1; // argc > 2 ? std::stoi(argv[2]) : 1;
    size_t work_group_size = 256; // argc > 3 ? std::stoi(argv[3]) : 256;

    Grid<3> grid{{10 * mesh_mod, 10, 10}};
    const auto &mesh = grid.mesh;

    std::cout << "Tetrahedras: " << mesh.template GetNumEntities<3>() << "\nIterations: " << iteration_mod << "\n";
    std::cout << "work_group_size: " << work_group_size << "\n";

    {
        constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

        auto buffers = MakeTuple<2>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<BaseType>(mesh, Dofs);
                fill_scalar(buffer, BaseType{10});
                return buffer;
            });

        auto ocl_buffers = MakeTuple<2>(
            [&](auto) {
                return ocl_runtime.GetBuffer<BaseType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<BaseType>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Forward Euler (scalar): {\n";
        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    {

        constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

        auto buffers = MakeTuple<6>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
                fill_scalar(buffer, BaseType { 1.0 });
                return buffer;
            });

        auto ocl_buffers = MakeTuple<6>(
            [&](auto) {
                return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<CoordinateType>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Runge Kutta: {\n";

        analyze(
            RungeKutta(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKutta.cl", "function_17"),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    {

        constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

        auto buffers = MakeTuple<6>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
                fill_random(buffer, BaseType{10});
                return buffer;
            });

        auto ocl_buffers = MakeTuple<6>(
            [&](auto) {
                return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<CoordinateType>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Runge Kutta (hand-written ocl): {\n";

        analyze(
            RungeKutta(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKutta-Hand.cl", "RK"),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    {

        constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

        auto buffers = MakeTuple<6>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
                fill_scalar(buffer, BaseType{10});
                return buffer;
            });

        auto ocl_buffers = MakeTuple<6>(
            [&](auto) {
                return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<CoordinateType>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Runge Kutta Stripped: {\n";

        analyze(
            RungeKuttaStripped(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKutta.cl", "function_17"),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }

    {

        constexpr auto Dofs = dof::MakeDofs<0, 0, 0, numVolNodes, 0>();

        auto buffers = MakeTuple<4>(
            [&](auto) {
                auto buffer = runtime.GetBuffer<CoordinateType>(mesh, Dofs);
                fill_scalar(buffer, BaseType{10});
                return buffer;
            });

        auto ocl_buffers = MakeTuple<4>(
            [&](auto) {
                return ocl_runtime.GetBuffer<CoordinateType>(mesh, Dofs, hpm_ocl.GetSVMAllocator<CoordinateType>());
            });

        assign_all(ocl_buffers, buffers);

        std::cout << "Volume: {\n";

        analyze(
            Volume(mesh, iteration_mod, buffers),
            VolumeOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size),
            iteration_mod);

        inequalities(ocl_buffers, buffers);
        std::cout << "}\n";
    }
}