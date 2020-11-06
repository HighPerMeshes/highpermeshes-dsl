// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_ACCESSDEFINITION_HPP
#define DSL_DATAACCESS_ACCESSDEFINITION_HPP

#include <type_traits>

#include <HighPerMeshes/dsl/buffers/LocalBuffer.hpp>
#include <HighPerMeshes/dsl/data_access/AccessMode.hpp>

namespace HPM
{
    //!
    //! \brief AccessDefinition is used to determine the access of data for a given buffer
    //!
    //! \see
    //! HPM::AccessDefinitionHelpers
    //!
    template <typename BufferT_, typename AccessPattern, size_t RequestedDimension_, AccessMode Mode_>
    struct AccessDefinition
    {
        using BufferT = BufferT_;
        static constexpr auto RequestedDimension = std::integral_constant<size_t, RequestedDimension_> {};

        static constexpr std::integral_constant<AccessMode, Mode_> Mode{};
        static constexpr bool UseCompileTimeDofs = BufferT::DofT::IsConstexprArray;

        AccessDefinition(BufferT* buffer, AccessPattern pattern, std::integral_constant<size_t, RequestedDimension_>, std::integral_constant<AccessMode, Mode_>) : buffer(buffer), pattern(pattern) {}

        //! Member to store the referenced buffer
        BufferT* buffer;
        AccessPattern pattern;
    };
} // namespace HPM

#endif