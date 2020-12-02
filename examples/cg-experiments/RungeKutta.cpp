#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include "RungeKutta.hpp"

using namespace HPM;

int main(int argc, char ** argv)
{
    auto [mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);

    Grid<3> grid{{10 * mesh_mod, 10, 10}};

    HPM::drts::Runtime runtime{HPM::GetBuffer{}};

    const auto &mesh = grid.mesh;

    auto buffers = PrepareSequentialBuffers<6, CoordinateType>(mesh, DGDofs, runtime);

    std::cout << "Runge Kutta\n";
    std::cout << "Mesh Size: " << mesh.GetNumEntities<3>() << " Tetraedrons\n";
    std::cout << "Iterations: " << iteration_mod << "\n";
    std::cout << "Dofs: " << numVolNodes << "\n";

    const auto range{ mesh.template GetEntityRange<3>() };

    HPM::SequentialDispatcher dispatcher{};
    
    dispatcher.Execute(
        HPM::iterator::Range{ size_t { iteration_mod } }, 
        RKKernel(range, buffers)
    );

}
