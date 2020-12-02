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

        // inequalities(buffers, ocl_buffers);

        // auto [measured_time, kernel_time] = RungeKuttaOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "RungeKutta.cl", "function_17");

        // size_t reads = 6;
        // size_t writes = 6;
        // size_t data_size = sizeof(CoordinateType);
        // size_t entries = Dofs.Get()[3] * mesh.GetNumEntities<3>();
        // size_t bytes = (reads + writes) * data_size * entries;

        // double avg_kernel_time = double { kernel_time } / iteration_mod; 

        // std::cout << "Avg Kernel Time: " << avg_kernel_time << " ns , data: " << bytes << " Bytes, " << double { bytes } / avg_kernel_time << " GB / s\n"; 

        std::cout << "}\n";
    }
}