#include "help.hpp"

#include "ForwardEuler.hpp"
#include "ForwardEulerOCL.hpp"

using namespace HPM;
int main(int argc, char **argv)
{
    auto [mesh_mod, iteration_mod, work_group_size] = get_args(argc, argv);
    auto [runtime, ocl_runtime, hpm_ocl, grid] = PrepareRuntimes(mesh_mod, iteration_mod, work_group_size);

    const auto &mesh = grid.mesh;

    {
        auto buffers = PrepareSequentialBuffers<2, EulerType>(mesh, EulerDofs, runtime);
        auto ocl_buffers = PrepareSequentialBuffers<2, EulerType>(mesh, EulerDofs, runtime);

        fill_scalar(std::get<0>(buffers), 1.0);
        fill_scalar(std::get<1>(buffers), 1.0);
        fill_scalar(std::get<0>(ocl_buffers), 1.0);
        fill_scalar(std::get<1>(ocl_buffers), 1.0);

        std::cout << "Dofs: " << NumEulerDofs << "\n";

        std::cout << "Forward Euler: {\n";

        analyze(
            ForwardEuler(mesh, iteration_mod, buffers),
            [&]() {
                std::fstream hpm_kernel_stream{"ForwardEulerSimple.cl"};
                std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());

                //get all platforms (drivers)
                std::vector<cl::Platform> all_platforms;
                cl::Platform::get(&all_platforms);
                if (all_platforms.size() == 0)
                {
                    std::cout << " No platforms found. Check OpenCL installation!\n";
                    exit(1);
                }
                cl::Platform default_platform = all_platforms[0];
                std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

                //get default device of the default platform
                std::vector<cl::Device> all_devices;
                default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
                if (all_devices.size() == 0)
                {
                    std::cout << " No devices found. Check OpenCL installation!\n";
                    exit(1);
                }
                cl::Device default_device = all_devices[0];
                std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

                cl::Context context({default_device});

                cl::Program::Sources sources;
                sources.push_back({hpm_kernel_string.c_str(), hpm_kernel_string.length()});

                cl::Program program(context, sources);
                if (program.build({default_device}) != CL_SUCCESS)
                {
                    std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
                    exit(1);
                }

                const auto range = mesh.GetNumEntities<0>();

                //create queue to which we will push commands for the device.
                cl::CommandQueue queue(context, default_device);

                cl::Buffer u(context, CL_MEM_READ_WRITE, sizeof(double) * std::get<0>(ocl_buffers).GetSize(), NULL);
                cl::Buffer u_d(context, CL_MEM_READ_WRITE, sizeof(double) * std::get<1>(ocl_buffers).GetSize(), NULL);

                queue.enqueueWriteBuffer(u, CL_TRUE, 0, sizeof(double) * std::get<0>(ocl_buffers).GetSize(), std::get<0>(ocl_buffers).GetData());
                queue.enqueueWriteBuffer(u_d, CL_TRUE, 0, sizeof(double) * std::get<1>(ocl_buffers).GetSize(), std::get<1>(ocl_buffers).GetData());

                cl::Kernel kernel(program, "function_1");
                kernel.setArg(0, u);
                kernel.setArg(1, u_d);

                auto measured = HPM::auxiliary::MeasureTime([&]() {
                   
                    for (size_t i = 0; i < iteration_mod; ++i)
                    {
                        queue.enqueueNDRangeKernel(
                            kernel,
                            cl::NullRange,
                            cl::NDRange(range * NumEulerDofs),
                            cl::NullRange);
                    }
                    queue.finish();

                });

                queue.enqueueReadBuffer(u, CL_TRUE, 0, sizeof(double) * std::get<0>(ocl_buffers).GetSize(), std::get<0>(ocl_buffers).GetData());
                queue.enqueueReadBuffer(u_d, CL_TRUE, 0, sizeof(double) * std::get<1>(ocl_buffers).GetSize(), std::get<1>(ocl_buffers).GetData());

                return measured.count();
            }(),
            iteration_mod);

        inequalities(buffers, ocl_buffers);

        std::cout << "}\n";
    }
}