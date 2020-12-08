#include "help.hpp"

#include "RungeKutta.hpp"
#include "RungeKuttaStripped.hpp"
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

        std::cout << "Runge Kutta Hand Written OCL: {\n";

         analyze(
            RungeKuttaStripped(mesh, iteration_mod, buffers),
            RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKuttaHandWritten.cl", "RK"),
            iteration_mod
        );

        std::cout << "}\n";
    }
}