# highpermeshes-dsl
Domain-specific language for matrix-free, unstructured grid algorithms

## Minimal Working Example

To implent an iterative solver in HighPerMeshes, the first step is to 
`#include <HighPerMeshes.hpp>` and define a runtime:

```
drts::Runtime hpm{
    GetBuffer{}
};
```
The runtime determines the configuration of HighPerMeshes. 
In a sequential example, there is no need for much configuration, but we have to
determine which buffers to use. The `GetBuffer` class determines that we use a 
normal `Buffer` to allocate data. 

The next step initializes a mesh.
For this purpose we define a mesh class that requires two types as its template parameter: 
* The first type tells us which dimensionality the mesh has and how to store the coordinates 
* The second type defines a class that defines the mesh topology, i.e. it specifies how nodes are connected and which  to each other. 

```
using CoordinateType = dataType::Vec<double, 3>;
using Mesh = mesh::Mesh<CoordinateType, HPM::entity::Simplex>;
const Mesh mesh = Mesh::template CreateFromFile<HPM::auxiliary::GambitMeshFileReader>(path_to_mesh_file);
```

The CoordinateType tells us which data type and which dimensionality to use for a given mesh. In this case we want to store the data in a three-dimensional vector of type double.
We currently provide the `Simplex` class to define meshes for simplexes of any dimensionality.
The `CreateFromFile` function uses a specific file reader to instantiate a mesh. In this case, it creates a mesh from a GAMBIT NEUTRAL FILE.

The next step is to instantiate a buffer. For this purpose, we require the notion of "degrees of freedom", which we also call dofs.
They allow us to associate entities of the mesh with space in a buffer:
```
constexpr auto dofs= dof::MakeDofs<0, 0, 0, 10>();
```
In this example, we define 10 degrees of freedom for each cell in the mesh. 
Then we construct a buffer:
```
auto buffer{
    hpm.GetBuffer<int>(mesh, dofs)
};
```
Independent of technology used, we can call the runtime's GetBuffer function to allocate data of type int for each entity in the mesh corresponding to the specified degrees of freedom.
In this case, we request a buffer of `int`s.

The last step is to actually define algorithms on mesh entities. 
For this, we first define a dispatcher that is used to dispatch kernels to some parallelization strategy:
```
SequentialDispatcher dispatcher;
```
In this example, we just execute each kernel sequentially.
With this dispatcher we can execute a list of solver steps:

```
dispatcher.Execute(
    iterator::Range{100}, 
    ForEachEntity(
        mesh.GetEntityRange<Mesh::CellDimension>(),
        std::tuple(Write(Cell(buffer))),
        /* kernel */
);
```
In this example, we want to repeat all specified loops 100 times. The `Range` parameter can be omitted, if the steps are only to be executed once.
We also just execute one type of kernel. `ForEachEntity` allows us to iterate over all entities of a certain range that is given by the first parameter.
The function `mesh.GetEntityRange<Mesh::CellDimension>` specifies that we want to iterate over each cell in the mesh.
Furthermote, the tuple given as the second paramter determines which data access we require in the loop. In this example, we want to have write access to the dofs associated with cells.

The last parameter is the computation for the kernel. For this kernel type, it always has three parameters:
* cell is the current entity considered, i.e. in this case it is one of the entities in `mesh.GetEntityRange<Mesh::CellDimension>()`
* step specifies the current time-step, i.e. in this case it is in the range [0, 100)
* local_view is used to access the local representation of the passed loops. 
```            
[&](const auto &cell, auto step, auto local_view) {
    auto& buffer_access = dof::GetDofs<Mesh::CellDimension>(std::get<0>(local_view));
    buffer_access[0] = 1;
})
```
In this example, `bufferAccess` is a translation of the local view into something useful. It gets the dofs for the currently considered cell for the first entry in the tuple we have defined before.
We can now use the bufferAccess with normal array operations. This array has the size of dofs we have defined before. In this case, it is accessible for indices in the range [0, 10).

Here is the comple example:
```
#include <HighPerMeshes.hpp>

using namespace HPM;

int main()
{

    drts::Runtime hpm{
        GetBuffer{}
    };

    const userInterfaceCFG::ConfigParser CFG("config.cfg");
    const std::string meshFile = CFG.GetValue<std::string>("MeshFile"); 

    using CoordinateType = dataType::Vec<double, 3>;
    using Mesh = mesh::Mesh<CoordinateType, HPM::entity::Simplex>;
    const Mesh mesh = Mesh::template CreateFromFile<HPM::auxiliary::GambitMeshFileReader>(meshFile);

    constexpr auto dofs= dof::MakeDofs<0, 0, 0, 10>();

    auto buffer{
        hpm.GetBuffer<int>(mesh, dofs)
    };

    SequentialDispatcher dispatcher;

    dispatcher.Execute(
        iterator::Range{100}, 
        ForEachEntity(
            mesh.GetEntityRange<Mesh::CellDimension>(),
            std::tuple(Write(Cell(buffer))),
            [&](const auto &cell, auto step, auto local_view) {
                auto& buffer_access = dof::GetDofs<Mesh::CellDimension>(std::get<0>(local_view));
                buffer_access[0] = 1;
            }
        )
    );
}
```
