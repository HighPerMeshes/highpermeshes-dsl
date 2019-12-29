// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DRTS_GETDISTRIBUTEDBUFFER_HPP
#define DRTS_GETDISTRIBUTEDBUFFER_HPP

#include <HighPerMeshes/dsl/buffers/DistributedBuffer.hpp>

namespace HPM
{
    //!
    //! \brief Provides a generator function for a DistributedBuffer
    //!
    //! \tparam Allocator The allocator used for the underlying buffer.
    //!
    //! GetBuffer provides a get function that generates a DistributedBuffer with the given parameters.
    //! It requires the runtime system to have a MyRank() function to determine the rank of the current compute node.
    //!
    //! \see DistributedBuffer
    //!
    template <template <typename> typename Allocator = std::allocator>
    struct GetDistributedBuffer
    {

        template <typename T, typename Runtime, typename MeshT, typename DofT>
        static auto Get(Runtime& runtime, const MeshT& mesh, const DofT dofs)
        {
            return DistributedBuffer<T, MeshT, DofT, Allocator<T>>(runtime.MyRank(), mesh, dofs, {});
        }
    };
} // namespace HPM

#endif