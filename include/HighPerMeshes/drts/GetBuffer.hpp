// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DRTS_GETBUFFER_HPP
#define DRTS_GETBUFFER_HPP

#include <HighPerMeshes/dsl/buffers/Buffer.hpp>

namespace HPM
{
    //!
    //! \brief Provides a generator function for a Buffer
    //!
    //! \tparam Allocator The allocator used for the underlying buffer.
    //!
    //! \see Buffer.hpp
    //!
    template <template <typename> typename Allocator = std::allocator>
    struct GetBuffer
    {
        template <typename T, typename MeshT, typename DofT>
        static auto Get(const MeshT& mesh, const DofT& dofs, const Allocator<T>& allocator = {})
        {
            return Buffer<T, MeshT, DofT, Allocator<T>>(mesh, dofs, allocator);
        }
    };
} // namespace HPM

#endif
