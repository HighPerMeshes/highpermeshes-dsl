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

        std::cout << "Volume : {\n";

        analyze(
            Volume(mesh, iteration_mod, buffers),
            VolumeOCL(mesh, iteration_mod, hpm_ocl, ocl_buffers, work_group_size, "Volume.cl", "function_30"),
            iteration_mod
        );

        std::cout << "}\n";
    }
}