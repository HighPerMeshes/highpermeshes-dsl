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

        std::cout << "Forward Euler: {\n";

        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size),
            iteration_mod
        );

        // inequalities(buffers, ocl_buffers);

        // auto [measured_time, kernel_time] = ForwardEulerOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size);

        // size_t reads = 2;
        // size_t writes = 1;
        // size_t data_size = sizeof(double);
        // size_t entries = Dofs.Get()[0] * mesh.GetNumEntities<0>();
        // size_t bytes = (reads + writes) * data_size * entries;

        // double avg_kernel_time = double { kernel_time } / iteration_mod; 

        // std::cout << "Avg Kernel Time: " << avg_kernel_time << " ns , data: " << bytes << " Bytes, " << double { bytes } / avg_kernel_time << " GB / s\n"; 

        std::cout << "}\n";
    }
}