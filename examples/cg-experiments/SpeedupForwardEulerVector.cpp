#include "help.hpp"

#include "ForwardEuler.hpp"
#include "ForwardEulerOCL.hpp"

using namespace HPM;
int main(int argc, char **argv)
{
    auto [ mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);
    auto [ runtime, ocl_runtime, hpm_ocl, grid ] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);

    const auto &mesh = grid.mesh;

    {
        auto [buffers, ocl_buffers] = PrepareBuffers<2, CoordinateType>(mesh, EulerDofs, runtime, ocl_runtime, hpm_ocl);
        
        std::cout << "Dofs: " << NumEulerDofs << "\n";

        std::cout << "Forward Euler Vector: {\n";

        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "ForwardEulerVector.cl", "function_4"),
            iteration_mod
        );

        std::cout << "}\n";
    }
}