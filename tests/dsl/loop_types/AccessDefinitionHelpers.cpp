// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include "../../util/UnitCube.hpp"

#include <gtest/gtest.h>

#include <HighPerMeshes/common/ConstexprArray.hpp>
#include <HighPerMeshes/dsl/LoopTypes.hpp>
#include <HighPerMeshes/dsl/buffers/Buffer.hpp>
#include <HighPerMeshes/dsl/data_access/AccessDefinitionHelpers.hpp>
#include <HighPerMeshes/dsl/dispatchers/SequentialDispatcher.hpp>

#include <type_traits>

using namespace HPM;

//!
//! A number of tests regarding data layout of the simple patterns Cell, Face, Edge and Node
//! as well as tests regarding the correct associations for All, ContainingMeshElement and NeighboringMeshElementOrSelf
//!
class AccessDefinitionHelpers : public ::testing::Test
{
  public:
    UnitCube cube;

    const decltype(cube.mesh)& mesh{cube.mesh};

    Buffer<int, std::decay_t<decltype(cube.mesh)>, dataType::ConstexprArray<std::size_t, 1, 1, 1, 1, 0>, std::allocator<int>> field{mesh, {}, {}};

    SequentialDispatcher dispatcher;

    template <typename Op>
    auto for_each(std::size_t until, Op op)
    {
        for (std::size_t i = 0; i < until; ++i)
        {
            op(i);
        }
    }

    template <typename Op>
    auto for_each(std::size_t from, std::size_t until, Op op)
    {
        for (std::size_t i = from; i < until; ++i)
        {
            op(i);
        }
    }

    auto rangeEquals(std::size_t from, std::size_t until, int value)
    {
        for_each(from, until, [&](auto i) { EXPECT_EQ(field[i], value); });
    }

    auto emptyRange(std::size_t from, std::size_t until) { rangeEquals(from, until, 0); }
};

constexpr size_t CellDim = 3;
constexpr size_t FaceDim = 2;
constexpr size_t EdgeDim = 1;
constexpr size_t NodeDim = 0;

TEST_F(AccessDefinitionHelpers, Cell)
{
    dispatcher.Execute(ForEachEntity(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(Cell(field))), [](const auto&, const auto&, auto& lvs) {
        auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));
        cell[0] += 1;
    }));

    rangeEquals(0, cube.NumCells, 1);
    emptyRange(cube.NumCells, field.GetSize());
}

TEST_F(AccessDefinitionHelpers, Face)
{

    dispatcher.Execute(ForEachEntity(mesh.GetEntityRange<FaceDim>(), std::tuple(ReadWrite(Face(field))), [](const auto&, const auto&, auto& lvs) {
        auto& face = dof::GetDofs<FaceDim>(std::get<0>(lvs));
        face[0] += 1;
    }));

    auto from = cube.NumCells;
    emptyRange(0, from);
    rangeEquals(from, from + cube.NumFaces, 1);
    emptyRange(from + cube.NumFaces, field.GetSize());
}

TEST_F(AccessDefinitionHelpers, Edge)
{

    dispatcher.Execute(ForEachEntity(mesh.GetEntityRange<EdgeDim>(), std::tuple(ReadWrite(Edge(field))), [](const auto&, const auto&, auto& lvs) {
        auto& edge = dof::GetDofs<EdgeDim>(std::get<0>(lvs));
        edge[0] += 1;
    }));

    auto from = cube.NumCells + cube.NumFaces;
    emptyRange(0, from);
    rangeEquals(from, from + cube.NumEdges, 1);
    emptyRange(from + cube.NumEdges, field.GetSize());
}

TEST_F(AccessDefinitionHelpers, Node)
{

    dispatcher.Execute(ForEachEntity(mesh.GetEntityRange<NodeDim>(), std::tuple(ReadWrite(Node(field))), [](const auto&, const auto&, auto& lvs) {
        auto& node = dof::GetDofs<NodeDim>(std::get<0>(lvs));
        node[0] += 1;
    }));

    auto from = cube.NumCells + cube.NumFaces + cube.NumEdges;
    emptyRange(0, from);
    rangeEquals(from, field.GetSize(), 1);
}

TEST_F(AccessDefinitionHelpers, AllFromCell)
{

    dispatcher.Execute(ForEachEntity(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(All(field))),
                                     [](const auto&, const auto&, auto& lvs) {
                                         auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));
                                         cell[0] += 1;

                                         auto& face = dof::GetDofs<FaceDim>(std::get<0>(lvs));
                                         ForEach(4, [&](auto i) { face[i][0] += 1; });

                                         auto& edge = dof::GetDofs<EdgeDim>(std::get<0>(lvs));
                                         ForEach(6, [&](auto i) { edge[i][0] += 1; });

                                         auto& node = dof::GetDofs<NodeDim>(std::get<0>(lvs));
                                         ForEach(4, [&](auto i) { node[i][0] += 1; });
                                     }),
                       ForEachEntity(mesh.GetEntityRange<FaceDim>(), std::tuple{Read(Face(field))},
                                     [](const auto& entity, const auto&, auto& lvs) {
                                         auto containingCells = entity.GetTopology().GetIndicesOfAllContainingCells();

                                         auto& face = dof::GetDofs<FaceDim>(std::get<0>(lvs));

                                         // If the considered entity does not belong to the middle cell
                                         if (containingCells.size() == 1)
                                         {
                                             EXPECT_EQ(face[0], 1);
                                         }
                                         else
                                         {
                                             EXPECT_EQ(face[0], 2);
                                         }
                                     }),
                       ForEachEntity(mesh.GetEntityRange<EdgeDim>(), std::tuple{Read(Edge(field))},
                                     [](const auto& entity, const auto&, auto& lvs) {
                                         auto containingCells = entity.GetTopology().GetIndicesOfAllContainingCells();

                                         auto& edge = dof::GetDofs<EdgeDim>(std::get<0>(lvs));

                                         // If the considered entity does not belong to the middle cell
                                         if (containingCells.size() == 1)
                                         {
                                             EXPECT_EQ(edge[0], 1);
                                         }
                                         else
                                         {
                                             EXPECT_EQ(edge[0], 3);
                                         }
                                     }),
                       ForEachEntity(mesh.GetEntityRange<NodeDim>(), std::tuple{Read(Node(field))}, [](const auto& entity, const auto&, auto& lvs) {
                           auto containingCells = entity.GetTopology().GetIndicesOfAllContainingCells();

                           auto& node = dof::GetDofs<NodeDim>(std::get<0>(lvs));

                           // If the considered entity does not belong to the middle cell
                           if (containingCells.size() == 1)
                           {
                               EXPECT_EQ(node[0], 1);
                           }
                           else
                           {
                               EXPECT_EQ(node[0], 4);
                           }
                       }));
}

TEST_F(AccessDefinitionHelpers, NeighboringMeshElementOrSelf)
{

    dispatcher.Execute(
        ForEachIncidence<FaceDim>(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(NeighboringMeshElementOrSelf(field))),
                                                 [](const auto&, const auto&, const auto&, auto& lvs) {
                                                     auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));

                                                     cell[0] += 1;
                                                 })
                    //                              ,
                    //    ForEachEntity(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(Cell(field))), [](const auto&, const auto&, auto& lvs) {
                    //        auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));

                    //        // Must be 4.
                    //        // First case: the face lies on a boundary, in which case NeighboringMeshElementOrSelf returns the cell itself.
                    //        // Second case: Is neighboring the middle cell, gets incremented by one from the middle cell
                    //        // Third case: Is inside the middle cell, gets incremented by one of the 4 boundary cells
                    //        EXPECT_EQ(cell[0], 4);
                    //    })
                       );
}

TEST_F(AccessDefinitionHelpers, ContainingMeshElement)
{

    dispatcher.Execute(ForEachIncidence<FaceDim>(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(ContainingMeshElement(field))),
                                                 [](const auto&, const auto&, const auto&, auto& lvs) {
                                                     auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));

                                                     cell[0] += 1;
                                                 }),
                       ForEachEntity(mesh.GetEntityRange<CellDim>(), std::tuple(ReadWrite(Cell(field))), [](const auto&, const auto&, auto& lvs) {
                           auto& cell = dof::GetDofs<CellDim>(std::get<0>(lvs));

                           // Must be 4.
                           // ForEachIncidence iterates over each sub-entity, in this case faces, and faces are repeated
                           // for each cell. Each Cell has 4 faces.
                           EXPECT_EQ(cell[0], 4);
                       }));
}
