#include "help.hpp"

#include "RungeKutta.hpp"
#include "RungeKuttaOCL.hpp"

using namespace HPM;
int main(int argc, char **argv)
{
    auto [ mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);
    auto [ runtime, ocl_runtime, hpm_ocl, grid ] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);

    const auto &mesh = grid.mesh;

    {
        auto [buffers, ocl_buffers] = PrepareBuffers<6, CoordinateType>(mesh, DGDofs, runtime, ocl_runtime, hpm_ocl);

        std::cout << "Dofs: " << numVolNodes << "\n";

        std::cout << "Runge Kutta: {\n";

         analyze(
            RungeKutta(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKutta.cl", "function_17"),
            iteration_mod
        );

        std::cout << "}\n";
    }
}