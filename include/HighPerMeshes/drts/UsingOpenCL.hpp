#ifndef USINGOPENCL_HPP
#define USINGOPENCL_HPP

#include <stdexcept>
#include <string>
#include <map>
#include <any>
#include <list>

#include <HighPerMeshes/dsl/dispatchers/OpenCLDispatcher.hpp>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

namespace HPM
{
    class ProfCL
    {
    private:
        cl::Event event;
        cl::CommandQueue queue;

    public:
        ProfCL(cl::CommandQueue q, cl::Event &e) : event(e), queue(q) {}

        unsigned long elapsed_ns()
        {
            queue.finish();
            return event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        }
    };

    class OpenCLHandler
    {

    public:
        typedef std::string kernel_name_type;

    private:
        cl::Platform platform;
        std::vector<cl::Device> devices;
        cl::Context context;

        cl::CommandQueue default_queue;

        std::map<kernel_name_type, cl::Kernel> kernels;

        // SVM stuff
        std::list<std::any> allocators;

        std::set<void *> unmapped_svm_buffers;
        std::set<void *> seen_svm_buffers;

        bool do_svm_unmap(const void *svm_ptr)
        {
            bool buffer_is_seen_for_the_fist_time = seen_svm_buffers.insert((void *)svm_ptr).second;
            bool buffer_is_not_unmapped = unmapped_svm_buffers.insert((void *)svm_ptr).second;
            return (buffer_is_seen_for_the_fist_time || buffer_is_not_unmapped);
        }

        bool do_svm_map(const void *svm_prt)
        {
            size_t buffer_is_unmapped = unmapped_svm_buffers.erase((void *)svm_prt);
            return buffer_is_unmapped;
        }

    public:
        template <typename T>
        using SVMAllocator = cl::SVMAllocator<T, cl::SVMTraitCoarse<>>;

        OpenCLHandler(std::string cl_platform_name, std::string cl_device_name)
        {

            // select platform
            std::vector<cl::Platform> platforms;
            cl::Platform::get(&platforms);

            for (auto plat = platforms.begin(); plat != platforms.end();)
                if (plat->getInfo<CL_PLATFORM_NAME>().find(cl_platform_name) == std::string::npos)
                    platforms.erase(plat);
                else
                    ++plat;

            if (platforms.size() != 1)
                throw std::runtime_error("ocl platform error");

            cl::Platform platform = platforms[0];
            std::cout << "Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

            // select device/s
            platform.getDevices(CL_DEVICE_TYPE_DEFAULT, &devices);

            for (auto dev = devices.begin(); dev != devices.end();)
                if (dev->getInfo<CL_DEVICE_NAME>().find(cl_device_name) == std::string::npos)
                    devices.erase(dev);
                else
                    ++dev;

            // for now use just a single device
            if (devices.size() < 1)
                throw std::runtime_error("no ocl devices found");

            cl::Device device = devices[0];
            std::cout << "Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

            if (!(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER & device.getInfo<CL_DEVICE_SVM_CAPABILITIES>()))
                throw std::runtime_error("device does not support SVM coarse grain buffers");

            // create context
            context = cl::Context(devices);
            cl::Context defc = cl::Context::setDefault(context);
            if (defc != context)
                throw std::runtime_error("error setting default context");

            // create default queue
            default_queue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);
            cl::CommandQueue defq = cl::CommandQueue::setDefault(default_queue);
            if (defq != default_queue)
                throw std::runtime_error("error setting default queue");
        }

        void LoadKernelsFromBinary(std::string binFilename, std::vector<kernel_name_type> kernelNames)
        {

            std::cerr << "Loading OpenCL kernel binary from '" << binFilename << "' ...";

            std::ifstream bin_file(binFilename, std::ifstream::binary);

            bin_file.seekg(0, bin_file.end);
            size_t bin_file_size = bin_file.tellg();

            std::vector<unsigned char> buf(bin_file_size);

            bin_file.seekg(0, bin_file.beg);
            bin_file.read((char *)buf.data(), bin_file_size);

            std::cerr << " DONE" << std::endl;

            cl::Program::Binaries bins;
            bins.push_back(buf);

            cl_int prog_err;
            cl::Program program(context, devices, bins, NULL, &prog_err);
            program.build();

            // RATIONALE: in case a new binary/source is loaded the old kernels are erased
            kernels.clear();

            for (std::string kname : kernelNames)
                kernels.emplace(kname, cl::Kernel(program, kname.c_str()));
        }

        void LoadKernelsFromString(std::string source, std::vector<kernel_name_type> kernelNames)
        {

            std::vector<std::string> _s;
            _s.push_back(source);
            cl_int prog_err, krnl_err;

            cl::Program program(context, _s, &prog_err);

            try
            {
                program.build(devices);
            }
            catch (cl::Error &e)
            {
                std::cerr << "Error in program build.\n"
                          << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
                _Exit(1);
            }

            // RATIONALE: in case a new binary/source is loaded the old kernels are erased
            kernels.clear();

            for (std::string kname : kernelNames)
            {
                kernels.emplace(kname, cl::Kernel(program, kname.c_str(), &krnl_err));
                std::cerr << kname << " " << krnl_err << std::endl;
            }
        }

        const cl::Context &GetContext() const
        {
            return context;
        }

        cl::CommandQueue &GetDefaultQueue()
        {
            return default_queue;
        }

        template <typename T>
        const SVMAllocator<T> &GetSVMAllocator()
        {
            return std::any_cast<const SVMAllocator<T> &>(allocators.emplace_back(SVMAllocator<T>(context)));
        }

        template <typename T>
        void SetKernelArg(kernel_name_type kernelName, size_t argIdx, const T &value)
        {
            kernels.at(kernelName).setArg(argIdx, value);
        }

        template <typename T, typename MeshT, typename DofT>
        void SetKernelArg(kernel_name_type kernelName, const size_t argIdx, const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            kernels.at(kernelName).setArg(argIdx, (void *)buffer.GetData());
        }

        template <typename T, typename MeshT, typename DofT>
        void SetKernelArg(kernel_name_type kernelName, const size_t argIdx, const std::vector<T, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            kernels.at(kernelName).setArg(argIdx, (void *)buffer.data());
        }

        template <typename T, typename MeshT, typename DofT>
        void UnmapSVMBuffer(const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            if (do_svm_unmap(buffer.GetData()))
                default_queue.enqueueUnmapSVM((void *)buffer.GetData());
        }

        template <typename T>
        void UnmapSVMBuffer(const std::vector<T, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            if (do_svm_unmap(buffer.data()))
                default_queue.enqueueUnmapSVM((void *)buffer.data());
        }

        template <typename T, typename MeshT, typename DofT>
        void MapSVMBuffer(const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            if (do_svm_map(buffer.GetData()))
                default_queue.enqueueMapSVM((void *)buffer.GetData(), CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, sizeof(T) * buffer.GetSize());
        }

        template <typename T>
        void MapSVMBuffer(const std::vector<T, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            if (do_svm_map(buffer.data()))
                default_queue.enqueueMapSVM((void *)buffer.data(), CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, sizeof(T) * buffer.size());
        }

        ProfCL EnqueueKernel(kernel_name_type kernelName, size_t global_wi = 1)
        {
            cl::Event kernel_event;
            default_queue.enqueueNDRangeKernel(kernels.at(kernelName), cl::NullRange, cl::NDRange(global_wi), cl::NDRange(1), NULL, &kernel_event);
            return ProfCL(default_queue, kernel_event);
        }
    };

    template <typename... KernelArg>
    class OpenCLKernelEnqueuer
    {

    private:
        using KernelArgs = std::tuple<KernelArg...>;

        template <typename T, typename MeshT, typename DofT>
        void Map(const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            ocl.MapSVMBuffer(buffer);
        }

        template <typename T>
        void Map(const std::vector<T, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            ocl.MapSVMBuffer(buffer);
        }

        template <typename T, typename MeshT, typename DofT>
        void Unmap(Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            ocl.UnmapSVMBuffer(buffer);
        }

        template <typename T>
        void Unmap(std::vector<T, OpenCLHandler::SVMAllocator<T>> &buffer)
        {
            ocl.UnmapSVMBuffer(buffer);
        }

        template <typename NOP_T>
        void Map(NOP_T /* not_used */) {}
        template <typename NOP_T>
        void Unmap(NOP_T /* not_used */) {}

    public:
        OpenCLKernelEnqueuer(OpenCLHandler &_ocl, OpenCLHandler::kernel_name_type _kernelName, KernelArgs _ka, size_t _wi_global_size = 1)
            : ocl(_ocl), kernelName(_kernelName), kernel_arguments(_ka), wi_global_size(_wi_global_size){};

        OpenCLHandler &ocl;
        OpenCLHandler::kernel_name_type kernelName;
        KernelArgs kernel_arguments;
        size_t wi_global_size;

        void unmap()
        {
            std::apply([this](auto &&... arg) { (Unmap(arg), ...); }, kernel_arguments); // eventually unmap the svm buffers
        }

        void enqueue()
        {
            std::apply([this](auto &&... arg) { size_t arg_id(0); ((ocl.SetKernelArg(kernelName, arg_id++, arg)), ...); }, kernel_arguments);
            ocl.EnqueueKernel(kernelName, wi_global_size);
        };

        void map()
        {
            std::apply([this](auto &&... arg) { (Map(arg), ...); }, kernel_arguments); // eventually map the svm buffers
        }

        template <typename... AdditionalArgs>
        OpenCLKernelEnqueuer<KernelArg..., AdditionalArgs...> with(std::tuple<AdditionalArgs...> additional_args)
        {
            return {ocl, kernelName, std::tuple_cat(kernel_arguments, std::move(additional_args)), wi_global_size};
        }

        OpenCLKernelEnqueuer<> clear()
        {
            return {ocl, kernelName, std::tuple{}, wi_global_size};
        }
    };

} // namespace HPM

#endif
