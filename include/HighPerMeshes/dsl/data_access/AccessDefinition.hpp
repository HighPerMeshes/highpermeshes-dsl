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
    template <typename BufferT_, typename AccessPattern, typename DofRequest, AccessMode Mode_>
    struct AccessDefinition
    {
        using BufferT = BufferT_;

        static constexpr std::integral_constant<AccessMode, Mode_> Mode{};
        static constexpr DofRequest Dofs{};
        static constexpr bool UseCompileTimeDofs = BufferT::DofT::IsConstexprArray;

        // \todo { Remove / Resolve this macro. Template parameter? - Stefan G. 2.12.19 }
#if defined(HPM_LOCAL_BUFFER_COPY_MODE)
        // \todo { Bug - FW 3.12.19 }
        template <std::size_t NumDofs = 0>
        using LocalBuffer = ::HPM::internal::LocalBuffer<BufferT, Mode_, true, NumDofs>;
#else
        template <std::size_t NumDofs = 0>
        using LocalBuffer = ::HPM::internal::LocalBuffer<BufferT, Mode_, false, NumDofs>;
#endif

        AccessDefinition(BufferT* buffer, AccessPattern pattern, DofRequest, std::integral_constant<AccessMode, Mode_>) : buffer(buffer), pattern(pattern) {}

        //! Member to store the referenced buffer
        BufferT* buffer;
        AccessPattern pattern;
    };
} // namespace HPM

#endif