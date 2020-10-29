// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DISPATCHERS_OPENCLDISPATCHER_HPP
#define DSL_DISPATCHERS_OPENCLDISPATCHER_HPP

#include <utility>
#include <tuple>
#include <type_traits>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/dispatchers/Dispatcher.hpp>

namespace HPM
{

    template <size_t Offset, typename Tuple, size_t... Indices>
    auto split_impl(Tuple &&tuple, std::index_sequence<Indices...> /* not_used */)
    {

        return std::tuple{
            std::get<Indices + Offset>(std::forward<Tuple>(tuple))...};
    }

    template <size_t Index, typename Tuple,
              typename FrontIndices = std::make_index_sequence<Index>,
              typename BackIndices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>> - Index>>
    auto tuple_split(Tuple &&tuple)
    {

        return std::pair{
            split_impl<0>(std::forward<Tuple>(tuple), FrontIndices{}),
            split_impl<Index>(std::forward<Tuple>(tuple), BackIndices{}),
        };
    }

    //!
    //! \brief Provides a class to execute OpenCL Kernels sequentially.
    //!
    //! SequentialDispatcher provides a sequential implementation for the Dispatcher base class that allows
    //! executing OpenCLKernel.
    //!
    //!
    class OpenCLDispatcher : public Dispatcher<OpenCLDispatcher>
    {
    public:
        //! Implementation of the dispatch function
        //! \see HPM::Dispatcher
        template <typename... OpenCLKernel, typename IntegerT>
        auto Dispatch(iterator::Range<IntegerT> range, OpenCLKernel &&... opencl_kernel)
        {
            (std::forward<OpenCLKernel>(opencl_kernel).unmap(), ...);

            for (auto step : range)
            {
                (
                    [&](auto& kernel) {
                        kernel.updateArg(2, step);
                        kernel.enqueue();
                    }(opencl_kernel)
                , ...);
            }

            (std::forward<OpenCLKernel>(opencl_kernel).map(), ...);
        }

    };
} // namespace HPM

#endif
