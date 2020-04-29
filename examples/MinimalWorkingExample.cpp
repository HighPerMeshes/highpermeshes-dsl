#include <HighPerMeshes.hpp>

#include <Grid.hpp>

using namespace HPM;

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

    Grid<3> grid {{ 100 , 100, 100 }};
    const auto& mesh = grid.mesh;

    //We store the CellDimension of the mesh because we're goint to use it more often later in the example.
    static constexpr auto CellDimension = 3;

    // We can determine what entities we want to iterate over by using the member functions of the mesh
    // `mesh.GetEntityRange` allows us to iterate over all entities of a certain dimension, in this case we want to iterate over each cell
    const auto AllCells{
        mesh.GetEntityRange<CellDimension>()
    };

    // Degrees of freedom, which are referred to `dofs` most of the time, allow us to associate entities of the mesh with space in a buffer.
    // In this example, we define just one degree of freedom for each face and each cell in the mesh 
    constexpr auto dofs= dof::MakeDofs<0, 0, 0, 1, 0>();

    // Here we allocate a buffer and see the benefits of leaving the buffer generation to the runtime system.
    // Independent of technology used, we can call the runtime's GetBuffer function to allocate data of type int for each entity in the mesh corresponding to the specified degrees of freedom.
    auto buffer{
        hpm.GetBuffer<int>(mesh, dofs)
    };

    // The dispatcher is used to dispatch kernels to some parallelization strategy.
    // In this case, we use a SequentialDispatcher to just execute the specified kernels
    SequentialDispatcher dispatcher;

    // The execute function executes a number of kernels for a number of time steps.
    // We're going to introduce both possible kernel types in this example
    
    auto time =
    auxiliary::MeasureTime([&](){
        dispatcher.Execute(
            // Repeat all specified loops 10 times. This can be omitted if the steps are only to be executed once
            iterator::Range{10}, 

            // First, we iterate over all entities. 
            // For this purpose, we have `ForEachEntity` that iterates over each entity in the specified range of entities
            ForEachEntity(

                //Iterate over the range of entities we have specified
                AllCells,

                //We further
                std::tuple(Write(Cell(buffer))),

                // Cell is the current entity, its
                // step specifies the repetition
                // local_view is used to access the local representation of the passed loops. They are ordered in the same way as the tuple specified just before it
                [&](const auto &, auto, auto local_view) {

                    auto& bufferAccess = dof::GetDofs<CellDimension>(std::get<0>(local_view));

                    const auto dof = 0;

                    bufferAccess[dof] = 1;

                })
        );
    });
    std::cout << "normal execute: " << time.count() << "ns\n";

    time =
    auxiliary::MeasureTime([&](){
        dispatcher.Execute(
            // Repeat all specified loops 10 times. This can be omitted if the steps are only to be executed once
            iterator::Range{10}, 

            // First, we iterate over all entities. 
            // For this purpose, we have `ForEachEntity` that iterates over each entity in the specified range of entities
            ForEachEntity(

                //Iterate over the range of entities we have specified
                AllCells,

                //We further
                std::tuple(Write(Cell(buffer))),

                // Cell is the current entity, its
                // step specifies the repetition
                // local_view is used to access the local representation of the passed loops. They are ordered in the same way as the tuple specified just before it
                [&](const auto &, auto, auto local_view) {

                    auto& bufferAccess = dof::GetDofs<CellDimension>(std::get<0>(local_view));

                    const auto dof = 0;

                    bufferAccess[dof] = 1;

                },
                
                HPM::internal::OpenMP_ForEachEntity<CellDimension>{}
            )
        );
    });
    std::cout << "openmp execute: " << time.count() << "ns\n";
}