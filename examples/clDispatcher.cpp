#include <HighPerMeshes.hpp>

#include <HighPerMeshes/drts/UsingOpenCL.hpp>

#include <Grid.hpp>
#include "MIDG2_DSL/data3dN03.hpp"
// #include "HelperFunctions.hpp"

using namespace HPM;

constexpr auto rk4 = std::array<std::array<double, 2>, 5>{
    std::array<double, 2>{0.0, 1432997174477.0 / 9575080441755.0},
    std::array<double, 2>{-567301805773.0 / 1357537059087.0, 5161836677717.0 / 13612068292357.0},
    std::array<double, 2>{-2404267990393.0 / 2016746695238.0, 1720146321549.0 / 2090206949498.0},
    std::array<double, 2>{-3550918686646.0 / 2091501179385.0, 3134564353537.0 / 4481467310338.0},
    std::array<double, 2>{-1275806237668.0 / 842570457699.0, 2277821191437.0 / 14882151754819.0}};



std::string clsource = 
"kernel void kernel_0(float global const * buffer1, float global const * buffer2, int s1, int s2, float global const * buffer3, float global const * buffer4)"
"{"
"    printf(\"global_id %i\\n\", get_global_id(0));\n"
"}"; 


int main()
{

    //The runtime determines the configuration of HighPerMeshes. 
    //The GetBuffer class determines that we use a normal buffer to allocate space 

    drts::Runtime hpm{
        GetBuffer{}
    };

    // We also provide a parser to read from a config file if needed. In this example we get the path to a GAMBIT neutral file.
    const auxiliary::ConfigParser CFG("config.cfg");
    const std::string meshFile = CFG.GetValue<std::string>("MeshFile"); 


    // The next step initializes a mesh
    // For this purpose we define a mesh class that needs two types as information: A CoordinateType that tells us which dimensionality the mesh has and how to store the coordinates and a topology class that can be used to define the mesh topology, i.e. how nodes are connected to each other. 
    // For this purpose we currently provide he Simplex class to define meshes for simplexes of any dimensionality

    // The CoordinateType tells us which data type and which dimensionality to use for a given mesh. 
    using CoordinateType = dataType::Vec<double, 3>;

    Grid<3> grid {{ 10 , 10, 10 }};
    const auto& mesh = grid.mesh;

    static constexpr auto CellDimension = 3;

    const auto AllCells{
        mesh.GetEntityRange<CellDimension>()
    };

    constexpr std::size_t order = 3;
    using DG = DgNodes<double, CoordinateType, order>;
    constexpr auto Dofs = dof::MakeDofs<0, 0, 0, DG::numVolNodes, 0>();

    auto buffer{
        hpm.GetBuffer<CoordinateType>(mesh, Dofs)
    };

    SequentialDispatcher dispatcher;

    dispatcher.Execute(
        HPM::ForEachEntity(
                AllCells,
                std::tuple(
                    Write(Cell(buffer)),
                    Write(Cell(buffer)),
                    Cell(buffer),
                    Cell(buffer),
                    Cell(buffer),
                    Cell(buffer)),
                [&](const auto &, const auto &iter, auto &lvs) {
                    const auto &RKstage = rk4[iter % 5];

                    auto &fieldH = std::get<0>(lvs);
                    auto &fieldE = std::get<1>(lvs);
                    auto &rhsH = std::get<2>(lvs);
                    auto &rhsE = std::get<3>(lvs);
                    auto &resH = std::get<4>(lvs);
                    auto &resE = std::get<5>(lvs);

                    HPM::ForEach(DG::numVolNodes, [&](const std::size_t n) {
                        resH[n] = RKstage[0] * resH[n] + /* timeStep * */ rhsH[n]; //!< residual fields
                        resE[n] = RKstage[0] * resE[n] + /* timeStep * */ rhsE[n];
                        fieldH[n] += RKstage[1] * resH[n]; //!< updated fields
                        fieldE[n] += RKstage[1] * resE[n];
                        assign_to_entries(rhsH[n], 0.0); //TODO
                        assign_to_entries(rhsE[n], 0.0);
                    });
                })
    );

    const std::string oclPlatformName = (true) ? "AMD Accelerated Parallel Processing" : "FPGA Emulation Platform"; 
    const std::string oclDeviceName = (true) ? "gfx1010" : "FPGA Emulation Device";

    drts::Runtime hpm_opencl{
        GetBuffer<OpenCLHandler::SVMAllocator>{}
    };

    OpenCLHandler _ocl(oclPlatformName, oclDeviceName);
    //_ocl.LoadKernelsFromBinary("rk.aocx", {"kernel_0"});
    _ocl.LoadKernelsFromString(clsource, {"kernel_0"});

    auto buffer_opencl_0{
       hpm_opencl.GetBuffer<float>(mesh, Dofs, _ocl.GetSVMAllocator<float>())
    };

    auto buffer_opencl_1{
       hpm_opencl.GetBuffer<float>(mesh, Dofs, _ocl.GetSVMAllocator<float>())
    };

    int size(1), step(1);
    auto kernel_args = std::tie(buffer_opencl_0, buffer_opencl_1, size, step, buffer_opencl_1, buffer_opencl_0);//, buffer_opencl, buffer_opencl, buffer_opencl, buffer_opencl);
    size_t wi_global_size = 2;

    auto kern_obj = HPM::OpenCLKernelEnqueuer { _ocl,"kernel_0", kernel_args, wi_global_size, 1 };    

    HPM::OpenCLDispatcher opencl_dispatcher;
    opencl_dispatcher.Execute(kern_obj, kern_obj);




}



