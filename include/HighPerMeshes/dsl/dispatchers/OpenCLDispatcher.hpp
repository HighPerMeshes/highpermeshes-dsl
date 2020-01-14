// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DISPATCHERS_OPENCLDISPATCHER_HPP
#define DSL_DISPATCHERS_OPENCLDISPATCHER_HPP

#include <utility>

#include <HighPerMeshes/common/Iterator.hpp>
#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>
#include <HighPerMeshes/dsl/dispatchers/Dispatcher.hpp>
#include <fstream>
#include <array>

#include <CL/cl.hpp>

namespace HPM
{


template<typename... List> struct FirstImpl;
template<typename Head, typename... Tail> struct FirstImpl<Head, Tail...> {
    using Type = Head;
};
template<typename... List> using First = typename FirstImpl<List...>::Type;

//!
//! \brief Provides a class to execute MeshLoops with OpenCL.
//!
//! OpenCLDispatcher provides an implementation for the Dispatcher base class that allows
//! executing MeshLoops with OpenCL.
//!
//! Usage:
//! Input: mesh, buffer
//! \code{.cpp}
//! OpenCLDispatcher { }.Execute(
//!     ForEachEntity(
//!     mesh.GetEntityRange<Mesh::CellDimension>(),
//!     std::tuple(Write(Cell(buffer))),
//!     [&] (const auto& cell, auto &&, auto lvs)
//!     {
//!         //...
//!     })
//! );
//! \endcode
//! \see
//! Dispatcher
//! \note
//! CRTP
//!
class OpenCLDispatcher : public Dispatcher<OpenCLDispatcher>
{
    auto platform_init() const
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        return platforms[0];
    }

    cl::Context init_context() const
    {

        cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0};
        return {
            CL_DEVICE_TYPE_GPU, context_properties};
    }

    auto program_init() const
    {
        std::ifstream sourceFile("kernels.cl");
        std::string sourceCode{
            "void kernel kernel0(global const int * mesh_info, int mesh_info_size, global int* buf){"
            " buf[get_global_id(0)]= get_global_id(0) + mesh_info[0];"
            "}"};

        /*std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>())*/
        cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

        // Make program of the source code in the context
        cl::Program program{context, source};

        // Build program for these specific devices
        auto code = program.build(devices);
        if (code != CL_SUCCESS)
        {

            for (cl::Device dev : devices)
            {
                // Check the build status
                cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
                if (status != CL_BUILD_ERROR)
                    continue;

                // Get the build log
                std::string name = dev.getInfo<CL_DEVICE_NAME>();
                std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
                std::cerr << "Build failed. Build log for " << name << ":" << std::endl
                          << buildlog << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }

        return program;
    }

public:
    OpenCLDispatcher() : platform{
                             platform_init()},
                         context{init_context()}, devices{context.getInfo<CL_CONTEXT_DEVICES>()}, queue{context, devices[0]}, program{program_init()}
    {
    }

    //! Implementation of the dispatch function
    //! \see HPM::Dispatcher
    template <typename... MeshLoops, typename IntegerT>
    auto Dispatch(iterator::Range<IntegerT> range, MeshLoops &&... mesh_loops)
    {

        constexpr auto CellDimension = std::decay_t<First<MeshLoops...>>::MeshT::CellDimension; 

        std::array<cl_int, CellDimension + 1> mesh_info;
        auxiliary::ConstexprFor<CellDimension + 1>(
            [&mesh_info, &mesh = std::get<0>(std::tuple { mesh_loops... }).entity_range.GetMesh()](auto i) {
                constexpr size_t index = i;
                mesh_info[index] = mesh.template GetNumEntities<index>();
            } 
        );
        const int mesh_info_buffer_size = mesh_info.size() * sizeof(cl_int);
        cl::Buffer mesh_info_buffer{context, CL_MEM_READ_ONLY, mesh_info_buffer_size};
        queue.enqueueWriteBuffer(mesh_info_buffer, CL_TRUE, 0, mesh_info_buffer_size, mesh_info.data());

        for (auto step : range)
        {
            ([self = this, step, &mesh_info_buffer, n = 0](auto &&mesh_loop) mutable {
                const auto &range = mesh_loop.entity_range;
                
                auto kernel_name = self->make_kernel_name(n);
                n++;

                constexpr size_t size = 10;

                cl::Kernel kernel{self->program, kernel_name.c_str()};
                cl::NDRange global{size};
                cl::NDRange local{1};

                kernel.setArg(0, mesh_info_buffer);
                kernel.setArg(1, cl_int { CellDimension + 1 });
                
                cl::Buffer buffer{self->context, CL_MEM_WRITE_ONLY, size * sizeof(int)};
                kernel.setArg(2, buffer);

                cl_int result = self->queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
                if(result != CL_SUCCESS) {
                    std::cerr << "Enqueue failed with error code " << result << "\n";
                    std::exit(EXIT_FAILURE);
                }

                std::array<int, size> array;
                self->queue.enqueueReadBuffer(buffer, CL_TRUE, 0, size * sizeof(int), array.data());

                for (auto &e : array)
                    std::cout << e << " ";
                std::cout << std::endl;
            }(std::forward<MeshLoops>(mesh_loops)),
             ...);
        }
    }

private:
    std::string make_kernel_name(size_t i)
    {
        return std::string{"kernel"} + std::to_string(i);
    }

    cl::Platform platform;
    cl::Context context;
    std::vector<cl::Device> devices;
    cl::CommandQueue queue;
    cl::Program program;
};

} // namespace HPM

#endif