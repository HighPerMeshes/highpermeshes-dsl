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

    template <typename GlobalBufferT, AccessMode Mode, bool CopyIn = false, std::size_t NumDofs = 0>
    class LocalBuffer
    {
        protected:
        using BlockT = typename GlobalBufferT::ValueT;
        using QualifiedBlockT = std::conditional_t<Mode == AccessMode::Read, const BlockT, BlockT>;

        GlobalBufferT* global_buffer;
        const std::size_t global_offset;

        public:
        LocalBuffer(GlobalBufferT* global_buffer, const std::size_t global_offset, [[maybe_unused]] const std::size_t size = 0) : global_buffer(global_buffer), global_offset(global_offset) {}

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

    template <typename GlobalBufferT, AccessMode Mode, std::size_t NumDofs>
    class LocalBuffer<GlobalBufferT, Mode, true, NumDofs> : public LocalBuffer<GlobalBufferT, Mode>
    {
        using Base = LocalBuffer<GlobalBufferT, Mode>;

        using BlockT = typename Base::BlockT;
        using QualifiedBlockT = typename Base::QualifiedBlockT;

        static constexpr bool UseCompileTimeDofs = GlobalBufferT::DofT::IsConstexprArray;
        static_assert(!UseCompileTimeDofs || (UseCompileTimeDofs && NumDofs > 0), "error: number of dofs is not provided.");

        using DataT = std::conditional_t<UseCompileTimeDofs, BlockT[NumDofs], std::vector<BlockT>>;

        using Base::global_buffer;
        using Base::global_offset;
        DataT data;

        public:
        LocalBuffer(GlobalBufferT* global_buffer, const std::size_t global_offset, [[maybe_unused]] const std::size_t size = 0) : Base(global_buffer, global_offset)
        {
            if constexpr (Mode == AccessMode::Write)
            {
                if constexpr (!UseCompileTimeDofs)
                {
                    data.resize(size);
                }
            }
            else
            {
                assert(global_buffer != nullptr);

                if constexpr (UseCompileTimeDofs)
                {
                    for (std::size_t i = 0; i < NumDofs; ++i)
                    {
                        data[i] = (*global_buffer)[global_offset + i];
                    }
                }
                else
                {
                    for (std::size_t i = 0; i < size; ++i)
                    {
                        data.push_back((*global_buffer)[global_offset + i]);
                    }
                }
            }
        }

        ~LocalBuffer() { Flush(); }

        inline auto operator[](const int index) -> QualifiedBlockT& { return data[index]; }

        inline auto operator[](const int index) const -> const QualifiedBlockT& { return data[index]; }

        inline auto Flush() const
        {
            if constexpr (Mode != AccessMode::Read)
            {
                assert(global_buffer != nullptr);

                if constexpr (UseCompileTimeDofs)
                {
                    for (std::size_t i = 0; i < NumDofs; ++i)
                    {
                        (*global_buffer)[global_offset + i] = data[i];
                    }
                }
                else
                {
                    for (std::size_t i = 0; i < data.size(); ++i)
                    {
                        (*global_buffer)[global_offset + i] = data[i];
                    }
                }
            }
        }
    };
} // namespace HPM::internal

#endif