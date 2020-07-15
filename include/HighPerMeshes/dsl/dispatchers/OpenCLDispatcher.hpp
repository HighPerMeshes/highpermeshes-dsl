// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DISPATCHERS_OPENCLDISPATCHER_HPP
#define DSL_DISPATCHERS_OPENCLDISPATCHER_HPP

#include <utility>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/dsl/dispatchers/Dispatcher.hpp>

namespace HPM
{
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
        auto Dispatch(iterator::Range<IntegerT> range, OpenCLKernel&&... opencl_kernel)
        {
            for (auto step : range)
            {
                (std::forward<OpenCLKernel>(opencl_kernel).enqueue() ,...);
            }
        }
    };
} // namespace HPM

#endif
