// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

#include <HighPerMeshes/common/DataTypes.hpp>
#include <HighPerMeshes/drts/GetBuffer.hpp>
#include <HighPerMeshes/dsl/data_access/AccessDefinition.hpp>
#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsAccessDefinitions.hpp>

TEST(IsDataAccesses, is_data_accesses)
{
    using CoordinateType = HPM::dataType::Coord3D;
    using Mesh = HPM::mesh::Mesh<CoordinateType, HPM::entity::Simplex>;
    using BufferT = HPM::Buffer<CoordinateType, Mesh, HPM::dataType::Dofs<0, 0, 0, 4>>;

    using TestType = int;

    using AccessDefinition1 = HPM::AccessDefinition<BufferT, TestType, TestType, HPM::AccessMode::Accumulate>;
    using AccessDefinition2 = HPM::AccessDefinition<BufferT, TestType, TestType, HPM::AccessMode::Read>;
    using AccessDefinition3 = HPM::AccessDefinition<BufferT, TestType, TestType, HPM::AccessMode::ReadWrite>;

    static_assert(HPM::IsAccessDefinitions<std::tuple<AccessDefinition1>>);
    static_assert(HPM::IsAccessDefinitions<std::tuple<AccessDefinition1, AccessDefinition2>>);
    static_assert(HPM::IsAccessDefinitions<std::tuple<AccessDefinition1, AccessDefinition2, AccessDefinition3>>);
    static_assert(HPM::IsAccessDefinitions<std::tuple<AccessDefinition3>>);

    static_assert(not HPM::IsAccessDefinitions<std::tuple<AccessDefinition1, int>>);
    static_assert(not HPM::IsAccessDefinitions<std::tuple<AccessDefinition1, BufferT, Mesh>>);
    static_assert(not HPM::IsAccessDefinitions<std::tuple<BufferT>>);
    static_assert(not HPM::IsAccessDefinitions<AccessDefinition1>);
}
