#include "help.hpp"

#include "Volume.hpp"
#include "VolumeOCL.hpp"

using namespace HPM;
int main(int argc, char **argv)
{
    auto [ mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);
    auto [ runtime, ocl_runtime, hpm_ocl, grid ] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);

    const auto &mesh = grid.mesh;

    {
        auto [buffers, ocl_buffers] = PrepareBuffers<4, CoordinateType>(mesh, DGDofs, runtime, ocl_runtime, hpm_ocl);
        
        std::cout << "Dofs: " << numVolNodes << "\n";

        std::cout << "Volume\n";

        auto kernel_time = VolumeOCLKernelTime(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "Volume.cl", "function_30");

        size_t reads = 2;
        size_t writes = 4;
        size_t data_size = sizeof(CoordinateType);
        size_t entries = DGDofs.Get()[3] * mesh.GetNumEntities<3>();
        size_t bytes = (reads + writes) * data_size * entries;

        double avg_kernel_time = double { kernel_time } / iteration_mod; 

        std::cout << "Avg Kernel Time: " << avg_kernel_time << " ns , data: " << bytes << " Bytes, " << double { bytes } / avg_kernel_time << " GB / s\n"; 

    }
}