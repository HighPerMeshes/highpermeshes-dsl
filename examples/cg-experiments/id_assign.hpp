#ifndef ID_ASSIGN_HPP
#define ID_ASSIGN_HPP

#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

auto IdAssign(HPM::OpenCLHandler &hpm_ocl)
{
    std::fstream hpm_kernel_stream{"id_assign.cl"};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());
    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {"foo"});

    std::vector<int, HPM::OpenCLHandler::SVMAllocator<int>> vec (257, hpm_ocl.GetSVMAllocator<int>());
    
    std::cout << "vec size: " << vec.size() << "\n";


    hpm_ocl.UnmapSVMBuffer(vec);

    hpm_ocl.SetKernelArg("foo", 0, vec);
    hpm_ocl.EnqueueKernel("foo", vec.size(), 256);

    hpm_ocl.MapSVMBuffer(vec);

    size_t i = 0;
    for(const auto& v : vec) {
        if(v != i) {
            std::cout << "Error at id: " << i << ", value: " << v << "\n";
        }
        i++;
    }

}


#endif /* ID_ASSIGN_HPP */
