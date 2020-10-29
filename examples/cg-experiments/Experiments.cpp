#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include "ForwardEuler.hpp"
#include "ForwardEulerOCL.hpp"
#include "RungeKutta.hpp"
#include "RungeKuttaOCL.hpp"
#include "Volume.hpp"
#include "VolumeOCL.hpp"

#include <cmath>

using namespace HPM;

int main(int argc, char **argv)
{

    const HPM::auxiliary::ConfigParser hpm_config_parser("config.cfg");
    const std::string hpm_ocl_platform_name = hpm_config_parser.GetValue<std::string>("oclPlatformName");
    const std::string hpm_ocl_device_name = hpm_config_parser.GetValue<std::string>("oclDeviceName");
    HPM::OpenCLHandler hpm_ocl(hpm_ocl_platform_name, hpm_ocl_device_name);
    
    auto analyze = [](auto seq_result, auto par_result, size_t iteration_mod) {
        
        double seq_time = static_cast<double>(seq_result);
        double par_time = static_cast<double>(par_result);
        
        std::cout   << "\tSequential Time = " << seq_result << " ns, Avgerage = " << seq_time / iteration_mod << " ns \n"
                    << "\tParallel Time = " << par_time << " ns, Average = " << par_time / iteration_mod << " ns \n"
                    << "\tSpeedup: " << seq_time / par_time << "\n";
    };

    for(size_t mesh_mod = 10; mesh_mod <= 1000; mesh_mod *= 10)
    {
        Grid<3> grid{{10 * mesh_mod, 10, 10}};
        const auto& mesh = grid.mesh;
            
        for(size_t iteration_mod = 100; iteration_mod <= 10000; iteration_mod *= 10) {

            std::cout << "Tetrahedras: " << mesh.template GetNumEntities<3>() << "\nIterations: " << iteration_mod << "\n";
            std::cout << "Forward Euler: {\n";

            analyze(
                ForwardEuler(mesh, iteration_mod),
                ForwardEulerOCL(mesh, iteration_mod, hpm_ocl),
                iteration_mod
            );
            std::cout << "}\n";

            std::cout << "Runge Kutta: {\n";
            analyze(
                RungeKutta(mesh, iteration_mod),
                RungeKuttaOCL(mesh, iteration_mod, hpm_ocl),
                iteration_mod
            );
            std::cout << "}\n";
            
            std::cout << "Volume: {\n";
            analyze(
                Volume(mesh, iteration_mod),
                VolumeOCL(mesh, iteration_mod, hpm_ocl),
                iteration_mod
            );
            std::cout << "}\n";
        }
    }

}