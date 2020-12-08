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
        auto [buffers, ocl_buffers] = PrepareBuffers<2, EulerType>(mesh, EulerDofs, runtime, ocl_runtime, hpm_ocl);
        
        std::cout << "Dofs: " << NumEulerDofs << "\n";

        std::cout << "Forward Euler Scalar: {\n";

        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "ForwardEuler.cl", "function_1"),
            iteration_mod
        );

        std::cout << "}\n";
    }
}