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
#include "id_assign.hpp"

#include <cmath>


int main(int argc, char **argv)
{

    size_t mesh_mod = argc > 1 ? std::stoi(argv[1]) : 1;
    size_t iteration_mod = argc > 2 ? std::stoi(argv[2]) : 1;
    size_t work_group_size = argc > 3 ? std::stoi(argv[3]) : 256;
    
    auto [ runtime, ocl_runtime, hpm_ocl, grid ] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);
    const auto& mesh = grid.mesh;

    {
        constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

        auto [buffers, ocl_buffers] = PrepareBuffers<2, BaseType>(mesh, Dofs, runtime, ocl_runtime, hpm_ocl);

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

        auto [buffers, ocl_buffers] = PrepareBuffers<6, CoordinateType>(mesh, Dofs, runtime, ocl_runtime, hpm_ocl);

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

        auto [buffers, ocl_buffers] = PrepareBuffers<6, CoordinateType>(mesh, Dofs, runtime, ocl_runtime, hpm_ocl);

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

        auto [buffers, ocl_buffers] = PrepareBuffers<6, CoordinateType>(mesh, Dofs, runtime, ocl_runtime, hpm_ocl);

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

        auto [buffers, ocl_buffers] = PrepareBuffers<4, CoordinateType>(mesh, Dofs, runtime, ocl_runtime, hpm_ocl);

        std::cout << "Volume: {\n";

        analyze(
            Volume(mesh, iteration_mod, buffers),
            VolumeOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size),
            iteration_mod);

        inequalities(ocl_buffers, buffers);
        std::cout << "}\n";
    }
}