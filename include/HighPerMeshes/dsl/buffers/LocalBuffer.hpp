// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_BUFFERS_LOCALBUFFER_HPP
#define DSL_BUFFERS_LOCALBUFFER_HPP

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

#include <HighPerMeshes/dsl/data_access/AccessMode.hpp>

namespace HPM::internal
{
    // A type that signals an invalid local buffer entry: see 'GetLocalBuffer() in LocalView.hpp'.
    class InvalidLocalBuffer
    {
    };

    template <typename GlobalBufferT, AccessMode Mode>
    class LocalBuffer
    {
        protected:
        using BlockT = typename GlobalBufferT::ValueT;
        using QualifiedBlockT = std::conditional_t<Mode == AccessMode::Read, const BlockT, BlockT>;

        GlobalBufferT* global_buffer;
        const std::size_t global_offset;

        public:
        LocalBuffer(GlobalBufferT* global_buffer, std::integral_constant<AccessMode, Mode>, size_t global_offset) : global_buffer(global_buffer), global_offset(global_offset) {}

        inline auto operator[](const int index) -> QualifiedBlockT&
        {
            assert(global_buffer != nullptr);

            return (*global_buffer)[global_offset + index];
        }

        inline auto operator[](const int index) const -> const QualifiedBlockT&
        {
            assert(global_buffer != nullptr);

            return (*global_buffer)[global_offset + index];
        }
    };

} // namespace HPM::internal

#endif