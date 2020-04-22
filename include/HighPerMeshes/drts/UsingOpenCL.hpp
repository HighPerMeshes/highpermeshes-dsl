#ifndef USINGOPENCL_HPP 
#define USINGOPENCL_HPP 

#include <stdexcept>
#include <string>
#include <map>
#include <any>
#include <list>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

namespace HPM
{

    class OpenCLHandler
    {

        private:

        typedef std::string kernel_name_type;

        cl::Platform platform;
        std::vector<cl::Device> devices;
        cl::Context context;

        cl::CommandQueue default_queue;

        std::list<std::any> allocators;           
        std::map<kernel_name_type, cl::Kernel> kernels;

        public:

        template <typename T> using SVMAllocator = cl::SVMAllocator<T, cl::SVMTraitCoarse<>>;

        OpenCLHandler(std::string cl_platform_name, std::string cl_device_name)
        {

            // select platform
            std::vector<cl::Platform> platforms;
            cl::Platform::get(&platforms);

            for(auto plat = platforms.begin(); plat != platforms.end(); )
                if(plat->getInfo<CL_PLATFORM_NAME>().find(cl_platform_name) == std::string::npos)
                    platforms.erase(plat);
                else ++plat;

            if(platforms.size() != 1) throw std::runtime_error("ocl platform error"); 

            cl::Platform platform = platforms[0];
            std::cout<<"Platform: "<<platform.getInfo<CL_PLATFORM_NAME>()<<std::endl;
          
            // select device/s
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

            for(auto dev = devices.begin(); dev != devices.end(); )
                if(dev->getInfo<CL_DEVICE_NAME>().find(cl_device_name) == std::string::npos)
                    devices.erase(dev);
                else ++dev;

            // for now use just a single device
            if(devices.size() != 1) throw std::runtime_error("ocl device error"); 
          
            cl::Device device = devices[0];
            std::cout<<"Device: "<<device.getInfo<CL_DEVICE_NAME>()<<std::endl;

            if(!(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER & device.getInfo<CL_DEVICE_SVM_CAPABILITIES>())) 
                throw std::runtime_error("device does not support SVM coarse grain buffers");

            // create context         
            context = cl::Context(devices);

            // create default queue 
            default_queue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);

        }

        void LoadKernelsFromBinary(std::string binFilename, std::vector<kernel_name_type> kernelNames)
        {

            std::cerr<<"Loading OpenCL kernel binary from '"<<binFilename<<"' ..."<<std::endl;

            std::ifstream bin_file(binFilename, std::ifstream::binary);

            bin_file.seekg (0, bin_file.end);
            size_t bin_file_size = bin_file.tellg();
          
            std::vector<unsigned char> buf(bin_file_size);

            bin_file.seekg (0, bin_file.beg);
            bin_file.read((char*) buf.data(), bin_file_size);
          
            cl::Program::Binaries bins;
            bins.push_back(buf);
          
            cl_int prog_err;
            cl::Program program(context, devices, bins, NULL, &prog_err);
            program.build();

            // RATIONALE: in case a new binary is loaded the old kernels are erased 
            kernels.clear();
         
            for(std::string kname : kernelNames)
                kernels.emplace(kname, cl::Kernel(program, kname.c_str()));

        }
 
        const cl::Context& GetContext() const
        {
            return context;
        }
 
        template <typename T>
        const SVMAllocator<T> & GetSVMAllocator()
        {
           return std::any_cast<const SVMAllocator<T> &>(allocators.emplace_back(SVMAllocator<T>(context)));
        }
       
        template <typename T>
        void SetKernelArg(kernel_name_type kernelName, size_t argIdx, const T& value)
        {
            kernels.at(kernelName).setArg(argIdx, value);
        }

        template <typename T, typename MeshT, typename DofT>
        void SetKernelArg(kernel_name_type kernelName, const size_t argIdx, const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>>& buffer)
        {
            kernels.at(kernelName).setArg(argIdx, (void*) buffer.GetData());
        }

        template <typename T, typename MeshT, typename DofT>
        void UnmapSVMBuffer(Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>> & buffer)
        {
            try
            {
                default_queue.enqueueUnmapSVM((void*) buffer.GetData());
            }
            catch (...) {};
        }

        template <typename T, typename MeshT, typename DofT>
        void MapSVMBuffer(const Buffer<T, MeshT, DofT, OpenCLHandler::SVMAllocator<T>>& buffer)
        {
            default_queue.enqueueMapSVM((void*) buffer.GetData(), sizeof(T)*buffer.GetSize(), CL_TRUE, CL_MAP_READ | CL_MAP_WRITE);
        }

        void EnqueueKernel(kernel_name_type kernelName)
        {
            default_queue.enqueueNDRangeKernel(kernels.at(kernelName),cl::NullRange,cl::NDRange(1),cl::NDRange(1), NULL, NULL);
        }

    };


} // namespace HPM

#endif

