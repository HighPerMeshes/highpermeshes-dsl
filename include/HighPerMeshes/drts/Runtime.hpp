/*
 * Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2018
 *
 * This file is part of HighPerMeshesDRTS, the HighPerMeshes distributed runtime
 * system.
 *
 * The HighPerMeshesDRTS is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * HighPerMeshesDRTS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * std::futr
 * You should have received a copy of the GNU General Public License
 * along with HighPerMeshesDRTS. If not, see <http://www.gnu.org/licenses/>.
 *
 * Runtime.hpp
 *
 */

#ifndef DRTS_RUNTIME_HPP
#define DRTS_RUNTIME_HPP

#include <HighPerMeshes/dsl/meta_programming/UtilTraits.hpp>
#include <type_traits>

namespace HPM::drts
{
    namespace internal
    {
        //!
        //! \name is_GetBuffer_invocable
        //!
        //! This template meta function determines if the `Get` function of T is invocable with the given Args for a given BlockT type
        //!
        template <typename T, typename BlockT, typename... Args>
        using InvokeGetBuffer = decltype(T::template Get<BlockT>(std::declval<Args&>()...));

        template <typename T, typename BlockT, typename... Args>
        constexpr bool GetBufferInvocable = IsSupported<InvokeGetBuffer, T, BlockT, Args...>;
    } // namespace internal

    //!
    //! This class provides functionality for HPM regarding the configurability of the system.
    //!
    //! \tparam  BufferHandler determines which Buffer to gernerate with the GetBuffer function.
    //! \tparam  Technologies The technologies available to the runtime system. This allows activating
    //!          or deactivating certain technologies and the corresponding overhead.
    //!
    //! \todo { The design of the RTS probably requires further discussion. I think the dependency injection approach of buffer_handler is far superior too the inheritance approach. - Stefan G. 11.07.2019 }
    //!
    template <typename BufferHandler, typename... Technologies>
    class Runtime : public Technologies...
    {
        using Self = Runtime<BufferHandler, Technologies...>;

        public:
        //!
        //! Invokes the default constructor for all technologies
        //!
        Runtime(BufferHandler buffer_handler) : Technologies()..., buffer_handler{buffer_handler} {}

        //!
        //! Provide ability to specify explicit constructor for every technology component
        //!
        template <typename... Args>
        Runtime(BufferHandler buffer_handler, Args&&... args) : Technologies(std::forward<Args>(args))..., buffer_handler{buffer_handler}
        {
        }

        ///!
        //! GetBuffer constructs the correct buffer given the Runtime's BufferHandler. The BufferHandler is invoked with
        //! the a pointer to the Runtime and the parameters if possible or just with the parameters otherwise.
        //!
        //! \param mesh The mesh associated with the generateed buffer
        //! \param dofs_per_entity The ith entry of dofs_per_entity specifies how many degrees of freedom the ith dimension has, i. e.
        //!                      how much space should be allocated for an element in the given dimension.
        //!
        template <typename BlockT, typename MeshT, typename DofT>
        auto GetBuffer(MeshT const& mesh, DofT dofs_per_entity)
        {
            if constexpr (internal::GetBufferInvocable<BufferHandler, BlockT, Self, MeshT const&, DofT>)
            {
                return buffer_handler.template Get<BlockT>(*this, mesh, dofs_per_entity);
            }
            else
            {
                return buffer_handler.template Get<BlockT>(mesh, dofs_per_entity);
            }
        }

        private:
        BufferHandler buffer_handler;
    };
} // namespace HPM::drts

#endif