#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

#include "ForwardEuler.hpp"

using namespace HPM;

int main(int argc, char ** argv)
{
    SequentialDispatcher dispatcher;
    
    auto [mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);

    Grid<3> grid{{10 * mesh_mod, 10, 10}};

    HPM::drts::Runtime runtime{HPM::GetBuffer{}};

    const auto &mesh = grid.mesh;

    auto buffers = PrepareSequentialBuffers<2, CoordinateType>(mesh, EulerDofs, runtime);

    std::cout << "Forward Euler\n";
    std::cout << "Mesh Size: " << mesh.GetNumEntities<3>() << " Tetraedrons, " << mesh.GetNumEntities<0>() << " vertices \n";
    std::cout << "Iterations: " << iteration_mod << "\n";
    std::cout << "Dofs: " << NumEulerDofs << "\n";

    const auto range{ mesh.template GetEntityRange<0>() };

    SequentialDispatcher{}.Execute(HPM::iterator::Range{ size_t { iteration_mod } }, ForwardEulerKernel(range, buffers));
}