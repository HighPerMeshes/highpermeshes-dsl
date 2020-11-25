#include "help.hpp"

#include "ForwardEuler.hpp"
#include "ForwardEulerOCL.hpp"

using namespace HPM;
int main(int argc, char **argv)
{

    size_t mesh_mod = argc > 1 ? std::stoi(argv[1]) : 1;
    size_t iteration_mod = argc > 2 ? std::stoi(argv[2]) : 1;
    size_t work_group_size = argc > 3 ? std::stoi(argv[3]) : 256;

    auto [runtime, ocl_runtime, hpm_ocl, grid] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);
    const auto &mesh = grid.mesh;

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
}