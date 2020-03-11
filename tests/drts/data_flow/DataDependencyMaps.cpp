// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

#include <algorithm>

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>

#include <HighPerMeshes/drts/data_flow/DataDependencyMaps.hpp>
#include <HighPerMeshes/dsl/data_access/AccessPatterns.hpp>
#include <HighPerMeshes/dsl/loop_types/loop_implementations/DefaultLoopImplementations.hpp>

#include "../../util/UnitCube.hpp"

using namespace HPM;
using namespace HPM::auxiliary;

//!
//! \class that separates the unit cube into evenly split local and global partitions and provides all functionality necessary to test the DataDependencyMaps
struct MockPartitionedMesh
{

    UnitCube cube;

    static constexpr size_t Cell = 3;
    static constexpr size_t Face = 2;
    static constexpr size_t Edge = 1;
    static constexpr size_t Node = 0;

    static constexpr std::array<size_t, 4> L1s{0, 1};
    static constexpr std::array<size_t, 4> L2s{0, 1, 2, 3};

    std::map<size_t, std::map<size_t, size_t>> dim2entity2local;
    std::map<size_t, std::map<size_t, std::set<size_t>>> dim2local2entities;

    auto GetNumL1Partitions() const { return 2; };

    auto L1PToL2P(size_t L1) const { return (L1 == 0) ? std::vector<size_t>{0, 1} : std::vector<size_t>{2, 3}; }

    MockPartitionedMesh()
    {

        // Initialize maps to get entity -> L2 and L2 -> entity by evenly dividing entities to L2 partitions
        ConstexprFor<Cell + 1>([& dim2entity2local = this->dim2entity2local, &dim2local2entities = this->dim2local2entities, &cube = this->cube](auto constant) {
            for (const auto& entity : cube.mesh.template GetEntities<constant.value>())
            {
                auto entityIndex = entity.GetTopology().GetIndex();
                auto L2Index = entityIndex * L2s.size() / cube.mesh.template GetNumEntities<constant.value>();
                dim2entity2local[constant.value][entityIndex] = L2Index;
                dim2local2entities[constant.value][L2Index].insert(entityIndex);
            }
        });
    }

    template <typename Entity>
    auto EntityToL2P(const Entity& e) const
    {
        return dim2entity2local.at(Entity::Dimension).at(e.GetTopology().GetIndex());
    }

    template <size_t Dimension = Cell>
    auto L2PToEntity(size_t L2) const
    {
        const std::vector<size_t> indices{dim2local2entities.at(Dimension).at(L2).begin(), dim2local2entities.at(Dimension).at(L2).end()};
        return cube.mesh.template GetEntityRange<Dimension>(indices).GetEntities();
    }

    //! \return the indices of entities of dimension `Dimension` in local partition `L2`
    auto L2PToEntityIndices(size_t L2, size_t Dimension = Cell) const { return dim2local2entities.at(Dimension).at(L2); }
};

class DataDependencyMapsTest : public testing::Test
{
  public:
    MockPartitionedMesh m;

    static constexpr size_t Cell = 3;
    static constexpr size_t Face = 2;
    static constexpr size_t Edge = 1;
    static constexpr size_t Node = 0;

    template <typename Map>
    auto entityAccess(Map&& map, size_t accessor, size_t accessed, size_t Dimension) const
    {
        return map.L2PHasAccessToL2PByEntity(accessor, accessed)[Cell - Dimension];
    }

    //! return if collection lhs and rhs are equal
    template <typename Collection>
    auto equals(Collection&& lhs, Collection&& rhs) const
    {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    auto L2Indices(size_t Dimension, size_t L2) { return m.L2PToEntityIndices(L2, Dimension); }
};

TEST_F(DataDependencyMapsTest, MockTesk)
{

    auto test = [index = 0](auto e) mutable {
        size_t entityIndex = e.GetTopology().GetIndex();
        ASSERT_EQ(entityIndex, index);
        index++;
    };

    size_t entityCount{0};
    for (auto L2 : m.L2s)
    {
        for (auto entity : m.L2PToEntity(L2))
        {
            test(entity);
            entityCount++;
        }
    }
    ASSERT_EQ(entityCount, 5);
}

template <size_t Dimension, typename DataDependencyMapsTest>
auto testSimplePattern(DataDependencyMapsTest&& test)
{

    drts::data_flow::DataDependencyMap<3> map{test.m, AccessPatterns::SimplePattern, ::HPM::internal::ForEachEntity<Dimension>{}};

    //! The correct access is that each entity has access directly to itself, i.e. on an entity basis each L2 partition has access to all entities in its partition.
    auto CorrectAccess = [&](auto L2) { EXPECT_TRUE(test.equals(test.entityAccess(map, L2, L2, Dimension), test.L2Indices(Dimension, L2))); };
    auto EmptyAccess = [&](auto accessor, auto accessed) { EXPECT_TRUE(test.entityAccess(map, accessor, accessed, Dimension).empty()); };

    for (auto L2 : map.L2PHasAccessToL2P(0))
    {
        ASSERT_TRUE(L2 == 0);
    }
    for (auto L2 : map.L2PHasAccessToL2P(1))
    {
        ASSERT_TRUE(L2 == 1);
    }
    for (auto L2 : map.L2PHasAccessToL2P(2))
    {
        ASSERT_TRUE(L2 == 2);
    }
    for (auto L2 : map.L2PHasAccessToL2P(3))
    {
        ASSERT_TRUE(L2 == 3);
    }

    for (size_t codimension = 0; codimension < 4; ++codimension)
    {
        for (auto accessorL2 : test.m.L2s)
        {
            for (auto accessedL2 : test.m.L2s)
            {
                if (codimension == test.Cell - Dimension)
                {
                    if (accessorL2 == accessedL2)
                    {
                        // entities need to access themselves
                        CorrectAccess(accessorL2);
                    }
                    else
                    {
                        // entities of the currently consided dimension should not access any other local partition
                        EmptyAccess(accessorL2, accessedL2);
                    }
                }
                else
                {
                    // no other dimension should have entries
                    EXPECT_TRUE(map.L2PHasAccessToL2PByEntity(accessorL2, accessedL2)[codimension].empty());
                }
            }
        }
    }
}

TEST_F(DataDependencyMapsTest, SimplePattern)
{

    testSimplePattern<Cell>(*this);
    testSimplePattern<Face>(*this);
    testSimplePattern<Edge>(*this);
    testSimplePattern<Node>(*this);
}

//! In this test we use the attribute that the last cell of the unit cube is in the middle, i.e. its connected to all other cells and all other cells are only connected to the middle one.
//! NeighboringMeshElementOrSelf with ForEachIncidence<2> will iterate over each face and will do one of the following things:
//! * If the current cell is not the middle one and the face lies on a boundary, it will access the current cell
//! * If the current cell is not the middle one and the face is connected to the middle one, it will access the middle cell
//! * If the currect cell is the middle one it will always access one of the other cells.
//! Furthermore, the last element will have its own local partition.
TEST_F(DataDependencyMapsTest, Neighboring)
{

    drts::data_flow::DataDependencyMap<3> map{m, AccessPatterns::NeighboringMeshElementOrSelfPattern, ::HPM::internal::ForEachIncidence<3, 2>{}};

    auto EmptyAccessWithDimension = [&](auto accessor, auto accessed, auto Dimension) { EXPECT_TRUE(entityAccess(map, accessor, accessed, Dimension).empty()); };
    auto EmptyAccess = [&](auto accessor, auto accessed) { EmptyAccessWithDimension(accessor, accessed, Cell); };
    //! The correct access is that a cell has entity to all entries in a certain L2 partition.
    auto CorrectAccess = [&](auto accessor, auto accessed) { EXPECT_TRUE(equals(entityAccess(map, accessor, accessed, Cell), L2Indices(Cell, accessed))); };

    for (auto L2 : map.L2PHasAccessToL2P(0))
    {
        ASSERT_TRUE(L2 == 0 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(1))
    {
        ASSERT_TRUE(L2 == 1 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(2))
    {
        ASSERT_TRUE(L2 == 2 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(3))
    {
        ASSERT_TRUE(L2 == 0 || L2 == 1 || L2 == 2);
    }

    EmptyAccess(0, 1);
    EmptyAccess(0, 2);
    EmptyAccess(1, 0);
    EmptyAccess(1, 2);
    EmptyAccess(2, 0);
    EmptyAccess(2, 1);
    EmptyAccess(3, 3);

    // 0 -> 0 & 3
    CorrectAccess(0, 0);
    CorrectAccess(0, 3);

    // 1 -> 1 & 3
    CorrectAccess(1, 1);
    CorrectAccess(1, 3);

    // 2 -> 2 & 3
    CorrectAccess(2, 2);
    CorrectAccess(2, 3);

    // 3 -> 0 & 1 & 2
    CorrectAccess(3, 0);
    CorrectAccess(3, 1);
    CorrectAccess(3, 2);

    // All other connections must be empty!
    for (size_t dimension = 0; dimension < 3; ++dimension)
    {
        for (auto accessorL2 : m.L2s)
        {
            for (auto accessedL2 : m.L2s)
            {
                EmptyAccessWithDimension(accessorL2, accessedL2, dimension);
            }
        }
    }
}

//! In this test we check if adding two DataDependencyMaps works correctly. We use the cell entity pattern and the neighboring entity pattern.
//! Therefore, the resulting DataDependencyMap should look exactly like in the neighboring case, but the middle element of the unit cube also has access to itself through the SimplePattern
TEST_F(DataDependencyMapsTest, AddMaps)
{

    drts::data_flow::DataDependencyMap<3> map{m, AccessPatterns::SimplePattern, ::HPM::internal::ForEachEntity<3>{}};
    map += drts::data_flow::DataDependencyMap<3>{m, AccessPatterns::NeighboringMeshElementOrSelfPattern, ::HPM::internal::ForEachIncidence<3, 2>{}};

    auto EmptyAccess = [&](auto accessor, auto accessed) { EXPECT_TRUE(entityAccess(map, accessor, accessed, Cell).empty()); };
    auto CorrectAccess = [&](auto accessor, auto accessed) { EXPECT_TRUE(equals(entityAccess(map, accessor, accessed, Cell), L2Indices(Cell, accessed))); };

    for (auto L2 : map.L2PHasAccessToL2P(0))
    {
        ASSERT_TRUE(L2 == 0 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(1))
    {
        ASSERT_TRUE(L2 == 1 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(2))
    {
        ASSERT_TRUE(L2 == 2 || L2 == 3);
    }
    for (auto L2 : map.L2PHasAccessToL2P(3))
    {
        ASSERT_TRUE(L2 == 0 || L2 == 1 || L2 == 2 || L2 == 3);
    }

    // No entity intersection
    EmptyAccess(0, 1);
    EmptyAccess(0, 2);

    EmptyAccess(1, 0);
    EmptyAccess(1, 2);

    EmptyAccess(2, 0);
    EmptyAccess(2, 1);

    // 0 -> 0 & 3
    CorrectAccess(0, 0);
    CorrectAccess(0, 3);

    // 1 -> 1 & 3
    CorrectAccess(1, 1);
    CorrectAccess(1, 3);

    // 2 -> 2 & 3
    CorrectAccess(2, 2);
    CorrectAccess(2, 3);

    // 3 -> 0 & 1 & 2 & 3
    CorrectAccess(3, 0);
    CorrectAccess(3, 1);
    CorrectAccess(3, 2);
    CorrectAccess(3, 3);

    // All other connections must be empty!
    for (size_t codimension = 1; codimension < 4; ++codimension)
    {
        for (auto accessorL2 : m.L2s)
        {
            for (auto accessedL2 : m.L2s)
            {
                ASSERT_TRUE(map.L2PHasAccessToL2PByEntity(accessorL2, accessedL2)[codimension].empty());
            }
        }
    }
}
