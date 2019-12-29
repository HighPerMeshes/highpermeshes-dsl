// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_DATATYPES_HPP
#define COMMON_DATATYPES_HPP

#include <HighPerMeshes/common/ConstexprArray.hpp>
#include <HighPerMeshes/common/Matrix.hpp>
#include <HighPerMeshes/common/Vec.hpp>

namespace HPM::dataType
{
    using Real = double;
    using Vec3D = Vec<Real, 3>;
    using Coord3D = Vec3D;
    using Mat3D = Matrix<Real, 3, 3>;

    template <std::size_t... Value>
    using Dofs = ConstexprArray<std::size_t, Value...>;
} // namespace HPM::dataType

#endif