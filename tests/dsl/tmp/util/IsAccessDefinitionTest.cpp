// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>
#include <tuple>

#include <HighPerMeshes/dsl/buffers/Buffer.hpp>
#include <HighPerMeshes/dsl/data_access/AccessDefinition.hpp>
#include <HighPerMeshes/dsl/entities/Simplex.hpp>
#include <HighPerMeshes/dsl/meshes/Mesh.hpp>
#include <HighPerMeshes/dsl/meta_programming/util/IsAccessDefinitions.hpp>

TEST(IsAccessDefinitionTest, is_access_definition)
{

    using TestType = int;

    using CoordinateType = HPM::dataType::Coord3D;
    using Mesh = HPM::mesh::Mesh<CoordinateType, HPM::entity::Simplex>;
    using BufferT = HPM::Buffer<CoordinateType, Mesh, HPM::dataType::Dofs<0, 0, 0, 4>>;

    static_assert(HPM::IsAccessDefinition<HPM::AccessDefinition<BufferT, TestType, 0, HPM::AccessMode::Read>>);

    static_assert(not HPM::IsAccessDefinition<BufferT>);
    static_assert(not HPM::IsAccessDefinition<Mesh>);
    static_assert(not HPM::IsAccessDefinition<std::tuple<BufferT>>);
}
