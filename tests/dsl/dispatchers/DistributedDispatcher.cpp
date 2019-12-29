// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

#include <HighPerMeshes/DRTS.hpp>
#include <HighPerMeshes/common/Vec.hpp>
#include <HighPerMeshes/drts/UsingDevice.hpp>
#include <HighPerMeshes/dsl/LoopTypes.hpp>
#include <HighPerMeshes/dsl/data_access/AccessDefinitionHelpers.hpp>
#include <HighPerMeshes/dsl/dispatchers/DistributedDispatcher.hpp>
#include <HighPerMeshes/dsl/meshes/PartitionedMesh.hpp>
#include <HighPerMeshes/third_party/metis/Partitioner.hpp>

#include "../../util/GaspiSingleton.hpp"
#include "../../util/Grid.hpp"

using namespace HPM;

using CoordinateType = dataType::Vec<double, 2>;

using namespace HPM;
using Mesh = HPM::mesh::PartitionedMesh<CoordinateType, HPM::entity::Simplex>;

using Partitioner = HPM::mesh::MetisPartitioner;

constexpr auto Dofs = ::HPM::dof::MakeDofs<0, 0, 1>();

struct DistributedDispatcherTest : public ::testing::Test
{

    GetDistributedBuffer<> bufferHandler;
    UsingDevice device;
    const Grid grid{10, 10};

    const Mesh mesh;

    auto GetBuffer() { return bufferHandler.Get<size_t>(GaspiSingleton::instance(), mesh, Dofs); }

    DistributedDispatcherTest()
        : mesh{[& grid = this->grid, &gaspi = GaspiSingleton::instance(), &device = this->device]() {
              auto nodes{grid.nodes};
              auto simplexes{grid.simplexes};
              return Mesh{std::move(nodes), std::move(simplexes), std::pair{gaspi.GetL1PartitionNumber(), device.GetL2PartitionNumber()}, gaspi.gaspi_runtime.rank().get(), Partitioner{}};
          }()}
    {
    }

    ~DistributedDispatcherTest() {}
};

TEST_F(DistributedDispatcherTest, NeighborPattern)
{
    auto& gaspi = GaspiSingleton::instance();

    auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

    auto buffer = GetBuffer();

    HPM::DistributedDispatcher body{gaspi.gaspi_context, gaspi.gaspi_segment, device};

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))), [](const auto& /* not_needed */, const auto& /* not_needed */, auto& local_view) {
        auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
        buffer[0] = -1;
    }));

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))),
                               [](const auto& /* not_needed */, const auto& /* not_needed */, auto& local_view) {
                                   auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                                   buffer[0] = 1;
                               }),
                 ForEachIncidence<Mesh::CellDimension - 1>(AllCells, std::tuple(Read(NeighboringMeshElementOrSelf(buffer))), [&](const auto& cell, const auto face, const auto& /* not_needed */, auto& local_view) {
                     auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                     EXPECT_EQ(buffer[0], 1) << "This L2: " << mesh.EntityToL2P(cell) << " Other L2: " << mesh.EntityToL2P(face.GetTopology().GetNeighboringCell());
                 }));
}

TEST_F(DistributedDispatcherTest, NeighborPatternSeparateExecutes)
{
    auto& gaspi = GaspiSingleton::instance();

    auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

    auto buffer = GetBuffer();

    HPM::DistributedDispatcher body{gaspi.gaspi_context, gaspi.gaspi_segment, device};

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))), [](const auto& /* not_needed */, const auto& /* not_needed */, auto& local_view) {
        auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
        buffer[0] = 1;
    }));

    body.Execute(ForEachIncidence<Mesh::CellDimension - 1>(AllCells, std::tuple(Read(NeighboringMeshElementOrSelf(buffer))),
                                                           [](const auto& /* not_needed */, const auto /* not_needed */, const auto& /* not_needed */, auto& local_view) {
                                                               auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                                                               EXPECT_EQ(buffer[0], 1);
                                                           }));
}

TEST_F(DistributedDispatcherTest, SimplePattern)
{

    auto& gaspi = GaspiSingleton::instance();

    auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

    auto buffer = GetBuffer();

    HPM::DistributedDispatcher body{gaspi.gaspi_context, gaspi.gaspi_segment, device};

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))), [](const auto& /* not_needed */, const auto& /* not_needed */, auto& local_view) {
        auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
        buffer[0] = -1;
    }));

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))),
                               [](const auto& e, const auto& /* not_needed */, auto& local_view) {
                                   auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                                   buffer[0] = e.GetTopology().GetIndex();
                               }),
                 ForEachEntity(AllCells, std::tuple(Read(Cell(buffer))), [](const auto& e, const auto& /* not_needed */, auto& local_view) {
                     auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                     EXPECT_EQ(buffer[0], e.GetTopology().GetIndex());
                 }));
}

TEST_F(DistributedDispatcherTest, SimplePatternSeparateExecution)
{

    auto& gaspi = GaspiSingleton::instance();

    auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

    auto buffer = GetBuffer();

    HPM::DistributedDispatcher body{gaspi.gaspi_context, gaspi.gaspi_segment, device};

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))), [](const auto& e, const auto& /* not_needed */, auto& local_view) {
        auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
        buffer[0] = e.GetTopology().GetIndex();
    }));

    body.Execute(ForEachEntity(AllCells, std::tuple(Read(Cell(buffer))), [](const auto& e, const auto& /* not_needed */, auto& local_view) {
        auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
        EXPECT_EQ(buffer[0], e.GetTopology().GetIndex());
    }));
}

TEST_F(DistributedDispatcherTest, ContainingMeshElement)
{

    auto& gaspi = GaspiSingleton::instance();

    auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

    auto buffer = GetBuffer();

    HPM::DistributedDispatcher body{gaspi.gaspi_context, gaspi.gaspi_segment, device};

    body.Execute(ForEachEntity(AllCells, std::tuple(Write(Cell(buffer))),
                               [](const auto& e, const auto& /* not_needed */, auto& local_view) {
                                   auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                                   buffer[0] = e.GetTopology().GetIndex();
                               }),
                 ForEachIncidence<Mesh::CellDimension - 1>(AllCells, std::tuple(Read(ContainingMeshElement(buffer))), [](const auto& e, const auto& /* not_needed */, const auto& /* not_needed */, auto& local_view) {
                     auto& buffer = dof::GetDofs<2>(std::get<0>(local_view));
                     EXPECT_EQ(buffer[0], e.GetTopology().GetIndex());
                 }));
}
