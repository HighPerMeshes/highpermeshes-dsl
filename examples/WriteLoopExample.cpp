#include <HighPerMeshes.hpp>
#include <WriteLoop.hpp>
#include <Grid.hpp>

int main() {

    using namespace HPM;

    std::mutex mutex;
    std::ofstream fstream { "output.txt" };

    drts::Runtime hpm{
        GetBuffer{}
    };
    
    Grid<3> grid {{ 10 , 10, 10 }};
    const auto& mesh = grid.mesh;

    constexpr auto cell_dofs= dof::MakeDofs<0, 0, 0, 1, 0>();
    auto cell_buffer { hpm.GetBuffer<double>(mesh, cell_dofs) };
    auto cells { mesh.GetEntityRange<3>() } ;

    SequentialDispatcher dispatcher;

    dispatcher.Execute(
        ForEachEntity(
        cells,
        std::tuple(Write(Cell(cell_buffer))),
        [&](const auto &, auto, auto local_view) {

            auto& bufferAccess = dof::GetDofs<3>(std::get<0>(local_view));

            const auto dof = 0;

            bufferAccess[dof] = 1;
        }),
        WriteLoop(mutex, fstream, cells, cell_buffer)
    );

    constexpr auto node_dofs= dof::MakeDofs<1, 0, 0, 0, 0>();
    auto node_buffer { hpm.GetBuffer<double>(mesh, node_dofs) };
    auto nodes { mesh.GetEntityRange<0>() } ;

    dispatcher.Execute(
        ForEachEntity(
        nodes,
        std::tuple(Write(Node(node_buffer))),
        [&](const auto &, auto, auto local_view) {

            auto& bufferAccess = dof::GetDofs<0>(std::get<0>(local_view));

            const auto dof = 0;

            bufferAccess[dof] = 1;
        }),
        WriteLoop(mutex, fstream, nodes, node_buffer)
    );

}