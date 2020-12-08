#ifndef HELP_HPP
#define HELP_HPP

#include <utility>
#include <tuple>
#include <HighPerMeshes.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include <random>
#include "Grid.hpp"



auto get_args(int argc, char ** argv) {

    size_t mesh_mod = argc > 1 ? std::stoi(argv[1]) : 1;
    size_t iteration_mod = argc > 2 ? std::stoi(argv[2]) : 1;
    size_t work_group_size = argc > 3 ? std::stoi(argv[3]) : 256;

    struct args {
            size_t mesh_mod;
            size_t iteration_mod; 
            size_t work_group_size;
    } res { mesh_mod, iteration_mod, work_group_size };

    return res; 
}

template <typename What, size_t... Indices>
auto make_tuple_impl(What &&what, std::index_sequence<Indices...> /* not_used */)
{
    return std::tuple{ std::forward<What>(what)(std::integral_constant<size_t, Indices>())... };
}

template <size_t HowMany, typename What, typename Indices = std::make_index_sequence<HowMany>>
auto MakeTuple(What &&what)
{
    return make_tuple_impl(std::forward<What>(what), Indices {}); 
}

using Real = double;
using Real3 = HPM::dataType::Vec<double, 3>;
using DerivativeBlock = std::array<Real3, 20>;
using Derivative = std::array<DerivativeBlock, 20>;

template<typename T>
auto get_random(T max) {
    static std::random_device r;
    static std::mt19937 gen(r());
    static std::uniform_real_distribution<T> uniform(0.0, max);
    return uniform(gen);
}

template<typename T>
void fill_random(HPM::dataType::Vec<T, 3>& vec, T max) {
    vec[0] = get_random(max);
    vec[1] = get_random(max);
    vec[2] = get_random(max);
}
 
template<typename T>
void fill_random(std::vector<HPM::dataType::Vec<T, 3>>& vec, T max) {
    for(auto& value : vec) {
        fill_random(value, max);
    }
}

template<typename Mesh, typename Dof, typename Allocator, typename T>
void fill_random(HPM::Buffer<HPM::dataType::Vec<T, 3>, Mesh, Dof, Allocator>& vec, T max) {
    for(auto& value : vec) {
        fill_random(value, max);
    }
}

template<typename T>
void fill_random(std::vector<T>& vec, T max) {
    for(auto& value : vec) {
        value = get_random(max);
    }
}

template<typename Mesh, typename Dof, typename Allocator, typename T>
void fill_random(HPM::Buffer<T, Mesh, Dof, Allocator>& vec, T max) {
    for(auto& value : vec) {
        value = get_random(max);
    }
}

template<typename T>
void fill_scalar(HPM::dataType::Vec<T, 3>& vec, T scalar) {
    vec[0] = scalar;
    vec[1] = scalar;
    vec[2] = scalar;
}


template<typename T>
void fill_scalar(std::vector<HPM::dataType::Vec<T, 3>>& vec, T scalar) {
    for(auto& value : vec) {
        fill_scalar(value, scalar);
    }
}

template<typename Mesh, typename Dof, typename Allocator, typename T>
void fill_scalar(HPM::Buffer<HPM::dataType::Vec<T, 3>, Mesh, Dof, Allocator>& vec, T scalar) {
    for(auto& value : vec) {
        fill_scalar(value, scalar);
    }
}

template<typename T>
void fill_scalar(std::vector<T>& vec, T scalar) {
    for(auto& value : vec) {
        value = scalar;
    }
}

template<typename Mesh, typename Dof, typename Allocator, typename T>
void fill_scalar(HPM::Buffer<T, Mesh, Dof, Allocator>& vec, T scalar) {
    for(auto& value : vec) {
        value = scalar;
    }
}

template <typename BufferA, typename BufferB>
void assign(BufferA &a, const BufferB &b)
{
#pragma omp parallel for
    for (size_t i = 0; i < a.GetSize(); ++i)
    {
        a[i] = b[i];
    }
}

template <typename Buffer>
void print(Buffer &buffer)
{
    std::cout << "{\n\t";
    for (const auto &v : buffer)
    {
        std::cout << v << " ";
    }
    std::cout << "\n}\n";
}

template <typename T>
auto compare_epsilon(T lhs, T rhs, T epsilon)
{
    return std::abs(lhs - rhs) < epsilon;
}

template <typename T>
auto compare_epsilon(HPM::dataType::Vec<T, 3> lhs, HPM::dataType::Vec<T, 3> rhs, double epsilon)
{
    auto res = lhs - rhs;

    return std::abs(res[0]) < epsilon && std::abs(res[1]) < epsilon && std::abs(res[2]) < epsilon;
}

template <typename T>
auto max_error(T current, T rhs)
{
    return std::max(current, std::abs(rhs));
}

template <typename T>
auto max_error(HPM::dataType::Vec<T, 3> current, HPM::dataType::Vec<T, 3> rhs)
{
    return HPM::dataType::Vec<T, 3>{
        std::max(current[0], std::abs(rhs[0])),
        std::max(current[1], std::abs(rhs[1])),
        std::max(current[2], std::abs(rhs[2]))};
}

template <typename BufferA, typename BufferB>
auto find_inequalities(const BufferA &a, const BufferB &b, double epsilon)
{

    struct Result
    {
        std::vector<size_t> differences;
        typename BufferA::ValueT max_error{};
    } result;

    for (size_t i = 0; i < a.GetSize(); ++i)
    {
        if (!compare_epsilon(a[i], b[i], epsilon))
        {
            result.differences.push_back(i);
        }

        result.max_error = max_error(result.max_error, a[i] - b[i]);
    }

    return result;
}

template <typename BufferA, typename BufferB>
auto print_inequalities(const std::vector<size_t> &inequalities, const BufferA &a, const BufferB &b)
{
    for (const auto &i : inequalities)
    {
        std::cout << "\t\t\tindex: " << i << ", a[i]: " << a[i] << ", b[i]:" << b[i] << ", difference: " << a[i] - b[i] << "\n";
    }
}

template <typename BuffersA, typename BuffersB>
auto assign_all(BuffersA &buffersA, const BuffersB &buffersB)
{
    HPM::auxiliary::ConstexprFor<std::tuple_size_v<std::decay_t<decltype(buffersA)>>>([&](auto index) {
        constexpr auto i = decltype(index)::value;
        assign(std::get<i>(buffersA), std::get<i>(buffersB));
    });
}

template <
    size_t Size,
    typename Datatype,
    typename Mesh,
    typename Dofs,
    typename SeqRuntime>
auto PrepareSequentialBuffers(const Mesh& mesh, const Dofs& dofs, SeqRuntime& seq_runtime) {
    return MakeTuple<Size>(
        [&](auto) {
            return seq_runtime.template GetBuffer<Datatype>(mesh, dofs);;
        });
}


template <
    size_t Size,
    typename Datatype,
    typename Mesh,
    typename Dofs,
    typename OclRuntime>
auto PrepareOCLBuffers(const Mesh& mesh, const Dofs& dofs, OclRuntime& ocl_runtime, HPM::OpenCLHandler& ocl_handler) {
    return MakeTuple<Size>(
        [&](auto) {
            return ocl_runtime.template GetBuffer<Datatype>(mesh, dofs, ocl_handler.GetSVMAllocator<Datatype>());
        });
}

template <
    size_t Size,
    typename Datatype,
    typename Mesh,
    typename Dofs,
    typename SeqRuntime,
    typename OClRuntime>
auto PrepareBuffers(
    const Mesh &mesh,
    const Dofs &dofs,
    SeqRuntime &seq_runtime,
    OClRuntime &ocl_runtime,
    HPM::OpenCLHandler &ocl_handler)
{

    std::tuple result {
        MakeTuple<Size>(
        [&](auto) {
            auto buffer = seq_runtime.template GetBuffer<Datatype>(mesh, dofs);
            fill_scalar(buffer, 1.0);
            return buffer;
        }),
        MakeTuple<Size>(
        [&](auto) {
            return ocl_runtime.template GetBuffer<Datatype>(mesh, dofs, ocl_handler.GetSVMAllocator<Datatype>());
        })
    };

    assign_all(std::get<1>(result), std::get<0>(result));

    return result;
}

auto analyze(double seq_result, double par_result, size_t iteration_mod)
{
    double seq_time = static_cast<double>(seq_result);
    double par_time = static_cast<double>(par_result);

    std::cout << "\tSequential Time = " << seq_result << " ns, Avgerage = " << seq_time / iteration_mod << " ns \n"
              << "\tParallel Time = " << par_time << " ns, Average = " << par_time / iteration_mod << " ns \n"
              << "\tSpeedup: " << seq_time / par_time << "\n";
}

template <typename BuffersA, typename BuffersB>
auto inequalities(const BuffersA &buffersA, const BuffersB &buffersB)
{
    std::cout << "\tinequalities: {\n";
    HPM::auxiliary::ConstexprFor<std::tuple_size_v<std::decay_t<decltype(buffersA)>>>([&](auto index) {
        constexpr auto i = decltype(index)::value;
        std::cout << "\t\tBuffer " << i << ":\n\t\t{\n";
        auto ineqs = find_inequalities(std::get<i>(buffersA), std::get<i>(buffersB), 1.0E-12);
        print_inequalities(ineqs.differences, std::get<i>(buffersA), std::get<i>(buffersB));
        std::cout << "max error: " << ineqs.max_error << "\n";
        std::cout << "\t\t}\n";
    });
    std::cout << "\t}\n";
}

auto PrepareRuntimes(size_t mesh_mod, size_t iteration_mod, size_t work_group_size) {
    
    const HPM::auxiliary::ConfigParser hpm_config_parser("config.cfg");
    const std::string hpm_ocl_platform_name = hpm_config_parser.GetValue<std::string>("oclPlatformName");
    const std::string hpm_ocl_device_name = hpm_config_parser.GetValue<std::string>("oclDeviceName");
    
    using namespace HPM;


    std::tuple result {
        drts::Runtime { GetBuffer{} },
        HPM::drts::Runtime {HPM::GetBuffer<HPM::OpenCLHandler::SVMAllocator>{}},
        HPM::OpenCLHandler (hpm_ocl_platform_name, hpm_ocl_device_name),
        Grid<3> {{10 * mesh_mod, 10, 10}}
    };


    std::cout << "Tetrahedras: " << std::get<3>(result).mesh.template GetNumEntities<3>() << "\nIterations: " << iteration_mod << "\n";
    std::cout << "work_group_size: " << work_group_size << "\n";

    return result;
}

auto load_kernel(HPM::OpenCLHandler &hpm_ocl, std::string file_name, std::string kernel_name)
{

    std::fstream hpm_kernel_stream{file_name};
    std::string hpm_kernel_string((std::istreambuf_iterator<char>(hpm_kernel_stream)), std::istreambuf_iterator<char>());
    hpm_ocl.LoadKernelsFromString(hpm_kernel_string, {kernel_name});
}


template <typename Kernel, typename AdditionalArg>
auto MeasureOCL(HPM::OpenCLHandler &hpm_ocl, std::string kernel_name, Kernel &kernel, size_t iteration_mod, size_t work_group_size, AdditionalArg&& arg)
{

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {
                       auto buffers = GetBuffers(kernel);
                       auto offsets = GetOffsets(kernel);
                       auto ocl_kernel = HPM::OpenCLKernelEnqueuer{hpm_ocl, kernel_name, std::tuple<unsigned long>{0}, kernel.entity_range.GetSize(), work_group_size}.with(buffers).with(offsets).with(std::forward<AdditionalArg>(arg));

                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{iteration_mod}, ocl_kernel);

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

template <typename Kernel>
auto MeasureOCL(HPM::OpenCLHandler &hpm_ocl, std::string kernel_name, Kernel &kernel, size_t iteration_mod, size_t work_group_size)
{

    return HPM::auxiliary::MeasureTime(
               [&]() {
                   {
                       auto buffers = GetBuffers(kernel);
                       auto offsets = GetOffsets(kernel);
                       auto ocl_kernel = HPM::OpenCLKernelEnqueuer{hpm_ocl, kernel_name, std::tuple<unsigned long>{0}, kernel.entity_range.GetSize(), work_group_size}.with(buffers).with(offsets);

                       HPM::OpenCLDispatcher{}.Dispatch(HPM::iterator::Range{iteration_mod}, ocl_kernel);

                       hpm_ocl.GetDefaultQueue().finish();
                   };
               })
        .count();
}

using BaseType = double;

using CoordinateType = HPM::dataType::Vec<BaseType, 3>;

constexpr auto numVolNodes = 20;
constexpr auto DGDofs = HPM::dof::MakeDofs<0,0,0,numVolNodes,0>();

constexpr auto rk4 = std::array<std::array<double, 2>, 5>{
    std::array<double, 2>{0.0, 0.1496590219992291},
    std::array<double, 2>{-0.4178904744998520, 0.3792103129996273},
    std::array<double, 2>{-1.1921516946426769, 0.8229550293869817},
    std::array<double, 2>{-1.6977846924715278, 0.6994504559491221},
    std::array<double, 2>{-1.5141834442571558, 0.1530572479681520 }};

constexpr auto derivative = Derivative {
    DerivativeBlock{Real3{Real{-3.0000000000000000e+00}, Real{-3.0000000000000000e+00}, Real{-3.0000000000000000e+00}},
                    Real3{Real{4.0450849718747302e+00}, Real{-3.5802794839105902e-16}, Real{1.3424148928403901e-15}},
                    Real3{Real{-1.5450849718747299e+00}, Real{4.0639432448880901e-16}, Real{-5.0237840281546102e-16}},
                    Real3{Real{4.9999999999999900e-01}, Real{-1.1033485125543401e-16}, Real{2.3498457460162499e-16}},
                    Real3{Real{-6.4601027964871695e-16}, Real{4.0450849718747302e+00}, Real{-3.6579136239513901e-16}},
                    Real3{Real{8.6151368970716803e-17}, Real{1.2550331560364000e-15}, Real{-2.9067657372004098e-16}},
                    Real3{Real{1.4588034090373301e-16}, Real{-1.9507874785395100e-16}, Real{-8.8232214685273503e-17}},
                    Real3{Real{3.2217835460249500e-16}, Real{-1.5450849718747399e+00}, Real{-7.8126440576178098e-16}},
                    Real3{Real{1.2171460603952801e-16}, Real{-4.9397890959476905e-16}, Real{2.2581893010046601e-16}},
                    Real3{Real{-4.3327171208775298e-17}, Real{5.0000000000000000e-01}, Real{0.0000000000000000e+00}},
                    Real3{Real{-7.5328497009953904e-16}, Real{-4.3888168193844901e-16}, Real{4.0450849718747399e+00}},
                    Real3{Real{4.3745938038242298e-16}, Real{-2.9515854864301598e-16}, Real{1.9981689672582201e-15}},
                    Real3{Real{-4.7663891659498900e-17}, Real{8.3926346559672101e-17}, Real{-9.9885204982266197e-16}},
                    Real3{Real{-8.2782092304419702e-16}, Real{-6.4983141276633403e-16}, Real{4.5916879626377996e-16}},
                    Real3{Real{1.1402056847584601e-15}, Real{-6.7339367032739001e-16}, Real{-6.3656695825111102e-16}},
                    Real3{Real{2.0202318328845501e-16}, Real{-7.8005789219742104e-16}, Real{0.0000000000000000e+00}},
                    Real3{Real{2.7186599575245300e-16}, Real{-1.9826186921551801e-16}, Real{-1.5450849718747399e+00}},
                    Real3{Real{-1.5006500680069801e-16}, Real{2.5003982099720400e-16}, Real{-1.7824310097090399e-16}},
                    Real3{Real{-1.2180098895175499e-16}, Real{2.5426583017538301e-16}, Real{7.5009719291724797e-16}},
                    Real3{Real{3.3073998916241201e-32}, Real{-3.3100455376630302e-17}, Real{4.9999999999999900e-01}}},
    DerivativeBlock{Real3{Real{-8.0901699437494801e-01}, Real{-7.0901699437494803e-01}, Real{-7.0901699437494803e-01}},
                    Real3{Real{5.7331670465990098e-16}, Real{-2.0000000000000000e+00}, Real{-2.0000000000000000e+00}},
                    Real3{Real{1.1180339887498900e+00}, Real{-1.9098300562505299e-01}, Real{-1.9098300562505299e-01}},
                    Real3{Real{-3.0901699437494701e-01}, Real{1.0000000000000001e-01}, Real{1.0000000000000001e-01}},
                    Real3{Real{-1.5980058898801799e-17}, Real{1.6180339887499000e+00}, Real{-1.1625435901742601e-16}},
                    Real3{Real{-3.6251347186248899e-18}, Real{2.7000000000000002e+00}, Real{-1.0173680080201500e-15}},
                    Real3{Real{-1.5543208479165401e-16}, Real{-1.9098300562505299e-01}, Real{3.9983697007543301e-16}},
                    Real3{Real{9.4415458310242108e-18}, Real{-1.3090169943749499e+00}, Real{-1.3021073429363000e-16}},
                    Real3{Real{1.1463486834866301e-16}, Real{-6.1803398874989501e-01}, Real{6.6674107440552100e-16}},
                    Real3{Real{-8.9387978485436406e-17}, Real{5.9999999999999998e-01}, Real{-2.9098800439454998e-16}},
                    Real3{Real{-9.6685240177059700e-17}, Real{4.8644451055870699e-17}, Real{1.6180339887499000e+00}},
                    Real3{Real{5.1354932792395798e-17}, Real{-1.5213948151591200e-17}, Real{2.7000000000000002e+00}},
                    Real3{Real{-4.2681144476213502e-18}, Real{7.3773011330069198e-17}, Real{-1.9098300562505299e-01}},
                    Real3{Real{-4.0637465295594600e-17}, Real{-4.0003241302191501e-18}, Real{-8.7397053157864101e-16}},
                    Real3{Real{-6.7082817987480105e-17}, Real{-8.9853143185963404e-16}, Real{-1.7505591351905501e-15}},
                    Real3{Real{7.8766046524307502e-17}, Real{-4.9325603350346497e-16}, Real{0.0000000000000000e+00}},
                    Real3{Real{-9.8839304549985604e-17}, Real{-1.6577184218839599e-16}, Real{-1.3090169943749499e+00}},
                    Real3{Real{1.1212619672659000e-16}, Real{3.0890053568389798e-16}, Real{-6.1803398874989501e-01}},
                    Real3{Real{6.6434460883019400e-18}, Real{1.6291508846156700e-16}, Real{3.7504859645862399e-16}},
                    Real3{Real{-1.7366985820036001e-17}, Real{-3.3100455376630302e-17}, Real{5.9999999999999898e-01}}},
    DerivativeBlock{Real3{Real{3.0901699437494801e-01}, Real{4.0901699437494698e-01}, Real{4.0901699437494798e-01}},
                    Real3{Real{-1.1180339887498900e+00}, Real{-1.3090169943749499e+00}, Real{-1.3090169943749499e+00}},
                    Real3{Real{4.4408920985006202e-16}, Real{-2.0000000000000000e+00}, Real{-2.0000000000000000e+00}},
                    Real3{Real{8.0901699437494701e-01}, Real{1.0000000000000001e-01}, Real{9.9999999999999298e-02}},
                    Real3{Real{-1.3506932994608899e-17}, Real{-1.9098300562505199e-01}, Real{-1.0250188168659999e-16}},
                    Real3{Real{-5.1787795214922999e-17}, Real{2.7000000000000002e+00}, Real{-1.7440594423202500e-15}},
                    Real3{Real{-2.3454261750660900e-17}, Real{1.6180339887498900e+00}, Real{2.9280427934384099e-16}},
                    Real3{Real{-6.2566824883851403e-17}, Real{-6.1803398874989401e-01}, Real{3.2552683573407502e-16}},
                    Real3{Real{1.2810000152189400e-16}, Real{-1.3090169943749499e+00}, Real{2.1366316333487699e-16}},
                    Real3{Real{-1.5529649999523699e-18}, Real{5.9999999999999998e-01}, Real{-2.4901858068379800e-16}},
                    Real3{Real{1.1306592720424399e-16}, Real{4.7681987519053997e-17}, Real{-1.9098300562505299e-01}},
                    Real3{Real{-1.5415227778452599e-16}, Real{-6.1861391694006199e-16}, Real{2.7000000000000002e+00}},
                    Real3{Real{-6.1846395013963903e-17}, Real{1.2042953173511601e-16}, Real{1.6180339887499000e+00}},
                    Real3{Real{7.0548585348897096e-18}, Real{-5.3994899252115697e-16}, Real{-1.3507569003574599e-15}},
                    Real3{Real{-6.8640381290805899e-17}, Real{2.8000649351226999e-16}, Real{-2.5502114584256798e-16}},
                    Real3{Real{1.2459359919773301e-16}, Real{9.8988467492971095e-18}, Real{9.7140438210515906e-17}},
                    Real3{Real{1.9931394687336400e-17}, Real{-8.3082311087632703e-16}, Real{-6.1803398874989501e-01}},
                    Real3{Real{7.2672770006480196e-17}, Real{4.3496989678312400e-16}, Real{-1.3090169943749499e+00}},
                    Real3{Real{6.2671149000019297e-19}, Real{3.9585321409320402e-16}, Real{7.5009719291724797e-16}},
                    Real3{Real{3.5550729771792799e-17}, Real{-1.0289688551719501e-31}, Real{5.9999999999999898e-01}}},
    DerivativeBlock{Real3{Real{-5.0000000000000000e-01}, Real{-5.0000000000000100e-01}, Real{-5.0000000000000100e-01}},
                    Real3{Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}},
                    Real3{Real{-4.0450849718747302e+00}, Real{-4.0450849718747302e+00}, Real{-4.0450849718747302e+00}},
                    Real3{Real{3.0000000000000000e+00}, Real{-4.4133940502173701e-16}, Real{-1.5446879175760800e-15}},
                    Real3{Real{-1.5057967739440501e-16}, Real{-8.0783671256458104e-17}, Real{-8.2376504969534298e-16}},
                    Real3{Real{-1.6529523414955399e-16}, Real{-2.5166188846482000e-15}, Real{2.2098793323907500e-16}},
                    Real3{Real{2.3384398302303001e-16}, Real{4.0450849718747399e+00}, Real{-7.2226065670491696e-16}},
                    Real3{Real{-1.6048204015329101e-17}, Real{0.0000000000000000e+00}, Real{1.2028184645752299e-15}},
                    Real3{Real{1.3342524297862301e-18}, Real{-1.5450849718747399e+00}, Real{-7.5550825855310704e-18}},
                    Real3{Real{-1.8091761897899999e-17}, Real{5.0000000000000000e-01}, Real{3.0058679889063300e-16}},
                    Real3{Real{7.2959839565973104e-16}, Real{1.0951633118766999e-15}, Real{-7.7626572923240104e-16}},
                    Real3{Real{-1.8701692597972702e-15}, Real{6.0062589688284000e-16}, Real{-3.4647605886036000e-16}},
                    Real3{Real{8.9202078498821297e-17}, Real{8.4103145058659196e-16}, Real{4.0450849718747399e+00}},
                    Real3{Real{1.3290500638130601e-16}, Real{-2.0053004044358201e-15}, Real{-1.2959397795148600e-15}},
                    Real3{Real{3.4775440696013601e-16}, Real{-1.0109997833215699e-15}, Real{8.9553172951491904e-16}},
                    Real3{Real{5.4495902169621302e-16}, Real{-5.7294378251379797e-16}, Real{-1.0685448203156699e-15}},
                    Real3{Real{4.6910779433668902e-16}, Real{-1.8474126321681600e-16}, Real{8.4386200510630000e-16}},
                    Real3{Real{4.7176791783539197e-17}, Real{-2.1111195087638799e-16}, Real{-1.5450849718747399e+00}},
                    Real3{Real{-2.8996215722278298e-16}, Real{3.9585321409320402e-16}, Real{1.5001943858345001e-15}},
                    Real3{Real{3.6367487903513701e-17}, Real{-7.3497775369424895e-32}, Real{4.9999999999999900e-01}}},
    DerivativeBlock{Real3{Real{-7.0901699437494703e-01}, Real{-8.0901699437494801e-01}, Real{-7.0901699437494803e-01}},
                    Real3{Real{1.6180339887498900e+00}, Real{3.8289858764701599e-16}, Real{7.8790295487489800e-17}},
                    Real3{Real{-1.3090169943749499e+00}, Real{6.8745120708123303e-19}, Real{-3.0342084095244502e-16}},
                    Real3{Real{5.9999999999999998e-01}, Real{1.1033485125543401e-16}, Real{3.0342084095244399e-16}},
                    Real3{Real{-2.0000000000000000e+00}, Real{2.8189478873908898e-16}, Real{-2.0000000000000000e+00}},
                    Real3{Real{2.7000000000000002e+00}, Real{-9.2349429430861899e-16}, Real{-2.8234943086296501e-15}},
                    Real3{Real{-6.1803398874989501e-01}, Real{0.0000000000000000e+00}, Real{1.4630906089046301e-16}},
                    Real3{Real{-1.9098300562505199e-01}, Real{1.1180339887499000e+00}, Real{-1.9098300562505299e-01}},
                    Real3{Real{-1.9098300562505299e-01}, Real{3.6695576141325701e-16}, Real{1.4113683131279100e-16}},
                    Real3{Real{1.0000000000000001e-01}, Real{-3.0901699437494801e-01}, Real{1.0000000000000001e-01}},
                    Real3{Real{-3.9102877588384502e-16}, Real{1.2478070882168401e-16}, Real{1.6180339887499000e+00}},
                    Real3{Real{-7.8275247521481402e-17}, Real{-7.1744642075314400e-17}, Real{0.0000000000000000e+00}},
                    Real3{Real{1.7040347967183599e-16}, Real{-2.2454655145381602e-16}, Real{-5.7990729458313699e-16}},
                    Real3{Real{2.7021480992586401e-16}, Real{-2.5879943997078398e-16}, Real{2.7000000000000002e+00}},
                    Real3{Real{1.2062172385764601e-15}, Real{2.9661779368991802e-16}, Real{-1.8500227224172898e-15}},
                    Real3{Real{6.9840151206983306e-17}, Real{3.3449909452584098e-16}, Real{-1.9098300562505299e-01}},
                    Real3{Real{1.5863454503442601e-16}, Real{-4.4517530974243897e-17}, Real{-1.3090169943749499e+00}},
                    Real3{Real{1.5863454503442601e-16}, Real{6.0965219664043695e-17}, Real{-9.1350741713816301e-17}},
                    Real3{Real{-2.4066938393207101e-17}, Real{-1.2880226822582400e-16}, Real{-6.1803398874989401e-01}},
                    Real3{Real{2.1167086236731499e-16}, Real{2.3085010670977699e-17}, Real{5.9999999999999898e-01}}},
    DerivativeBlock{Real3{Real{3.3333333333333398e-01}, Real{3.3333333333333298e-01}, Real{3.8888888888888901e-01}},
                    Real3{Real{-6.2112999374994105e-01}, Real{-7.2723166354163804e-01}, Real{-7.2723166354163704e-01}},
                    Real3{Real{6.2112999374994105e-01}, Real{-1.0610166979169600e-01}, Real{-1.0610166979169500e-01}},
                    Real3{Real{-3.3333333333333298e-01}, Real{1.3791856406929300e-16}, Real{5.5555555555555199e-02}},
                    Real3{Real{-7.2723166354163704e-01}, Real{-6.2112999374994105e-01}, Real{-7.2723166354163704e-01}},
                    Real3{Real{-1.3911344246271301e-15}, Real{-1.1054577863706699e-15}, Real{-1.5000000000000000e+00}},
                    Real3{Real{7.2723166354163804e-01}, Real{1.0610166979169600e-01}, Real{7.5593014793405899e-16}},
                    Real3{Real{-1.0610166979169600e-01}, Real{6.2112999374994105e-01}, Real{-1.0610166979169600e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{7.2723166354163804e-01}, Real{1.2702314818151199e-16}},
                    Real3{Real{-1.9171369943972798e-17}, Real{-3.3333333333333298e-01}, Real{5.5555555555555497e-02}},
                    Real3{Real{8.7092049233659103e-17}, Real{3.2871912768519901e-17}, Real{-1.0610166979169600e-01}},
                    Real3{Real{1.1699816712219700e-16}, Real{-1.5564248597125501e-16}, Real{1.5000000000000000e+00}},
                    Real3{Real{-7.0071141547210300e-18}, Real{-1.2216040407611401e-16}, Real{-1.0610166979169500e-01}},
                    Real3{Real{-3.8713149945490301e-17}, Real{-4.1096059118786297e-17}, Real{1.5000000000000000e+00}},
                    Real3{Real{1.4297682400275401e-16}, Real{-9.6761644578150202e-17}, Real{1.5000000000000000e+00}},
                    Real3{Real{-4.4543269007873701e-17}, Real{2.9308560691054198e-16}, Real{-1.0610166979169600e-01}},
                    Real3{Real{-6.7563020504027197e-18}, Real{1.7323084431853299e-16}, Real{-7.2723166354163704e-01}},
                    Real3{Real{9.8726448587884797e-17}, Real{6.7748093680245999e-17}, Real{-7.2723166354163804e-01}},
                    Real3{Real{-2.2527983016621200e-17}, Real{-1.0788738528515100e-16}, Real{-7.2723166354163704e-01}},
                    Real3{Real{-1.9217210201444499e-17}, Real{8.1675813172088497e-19}, Real{6.6666666666666596e-01}}},
    DerivativeBlock{Real3{Real{-5.9999999999999998e-01}, Real{-5.9999999999999898e-01}, Real{-5.9999999999999998e-01}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}},
                    Real3{Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{7.0901699437494603e-01}, Real{-1.0000000000000001e-01}, Real{-8.6888695363654599e-16}},
                    Real3{Real{6.1803398874989302e-01}, Real{6.1803398874989401e-01}, Real{6.1803398874989401e-01}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{2.0000000000000000e+00}, Real{2.0000000000000000e+00}, Real{4.3892718267138898e-16}},
                    Real3{Real{1.9098300562505299e-01}, Real{1.9098300562505199e-01}, Real{1.9098300562505299e-01}},
                    Real3{Real{1.9098300562505299e-01}, Real{1.3090169943749499e+00}, Real{-5.3527498704726199e-16}},
                    Real3{Real{-1.0000000000000001e-01}, Real{-4.0901699437494698e-01}, Real{1.0937844450658100e-16}},
                    Real3{Real{1.7333825474569000e-16}, Real{2.5211577461159202e-16}, Real{-2.9946283923529098e-16}},
                    Real3{Real{9.6336033270400894e-18}, Real{3.3047617982430899e-16}, Real{0.0000000000000000e+00}},
                    Real3{Real{-4.4366275644302002e-17}, Real{-1.9381981135927700e-16}, Real{1.6180339887499000e+00}},
                    Real3{Real{-1.9875969666725201e-16}, Real{-3.2075987603601500e-16}, Real{4.3829430180595602e-16}},
                    Real3{Real{-2.0444008279435000e-16}, Real{3.0533172658538501e-16}, Real{2.7000000000000002e+00}},
                    Real3{Real{1.4135782182357299e-16}, Real{-1.1784665647751699e-16}, Real{-1.9098300562505299e-01}},
                    Real3{Real{-1.5766612160156699e-16}, Real{2.1765901259038100e-16}, Real{4.2193100255315000e-16}},
                    Real3{Real{5.3299379675007998e-17}, Real{2.4402970024995299e-16}, Real{-1.3090169943749499e+00}},
                    Real3{Real{-5.2183370963279600e-17}, Real{9.3818243849442398e-17}, Real{-6.1803398874989401e-01}},
                    Real3{Real{1.2911590965328499e-16}, Real{-7.5292782729138902e-17}, Real{5.9999999999999898e-01}}},
    DerivativeBlock{Real3{Real{4.0901699437494798e-01}, Real{3.0901699437494801e-01}, Real{4.0901699437494798e-01}},
                    Real3{Real{-1.9098300562505199e-01}, Real{5.5167425627717200e-17}, Real{-3.4589115349284399e-16}},
                    Real3{Real{-6.1803398874989401e-01}, Real{5.5167425627717200e-17}, Real{3.6319982947090598e-17}},
                    Real3{Real{5.9999999999999998e-01}, Real{-5.5167425627717200e-17}, Real{-1.1033485125543401e-16}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-1.1180339887498900e+00}, Real{-1.3090169943749499e+00}},
                    Real3{Real{2.7000000000000002e+00}, Real{-8.7202972116012502e-16}, Real{-2.6160891634803699e-15}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-2.0502788505243400e-17}, Real{1.9507874785395100e-16}},
                    Real3{Real{-2.0000000000000000e+00}, Real{4.8026403647612599e-16}, Real{-2.0000000000000000e+00}},
                    Real3{Real{1.6180339887498900e+00}, Real{1.2702314818151199e-16}, Real{-5.1691364468309800e-16}},
                    Real3{Real{1.0000000000000001e-01}, Real{8.0901699437494701e-01}, Real{1.0000000000000001e-01}},
                    Real3{Real{3.2136787275216301e-16}, Real{4.6923727816666702e-17}, Real{-1.9098300562505199e-01}},
                    Real3{Real{-5.2692998815944700e-17}, Real{-4.1030824431274899e-16}, Real{-3.2706521026891700e-16}},
                    Real3{Real{-1.7088534319849800e-16}, Real{4.2590628073983099e-17}, Real{3.8660486305542501e-16}},
                    Real3{Real{-1.6657475356230201e-16}, Real{-4.5517752999611997e-17}, Real{2.7000000000000002e+00}},
                    Real3{Real{5.3650574834571200e-18}, Real{-1.9264956790163600e-16}, Real{-1.5914173956277801e-15}},
                    Real3{Real{-6.5610845868922498e-16}, Real{-4.2683286798663703e-17}, Real{1.6180339887498900e+00}},
                    Real3{Real{4.0277793292813600e-16}, Real{2.3909841007266801e-16}, Real{-6.1803398874989501e-01}},
                    Real3{Real{-1.9153069625014500e-17}, Real{-5.0979154182623002e-17}, Real{3.6540296685526501e-16}},
                    Real3{Real{1.9181243165156100e-16}, Real{2.3108574051574701e-16}, Real{-1.3090169943749499e+00}},
                    Real3{Real{-9.2659768549174801e-17}, Real{-1.4916711424873399e-17}, Real{5.9999999999999898e-01}}},
    DerivativeBlock{Real3{Real{-5.9999999999999998e-01}, Real{-5.9999999999999998e-01}, Real{-5.9999999999999998e-01}},
                    Real3{Real{6.1803398874989501e-01}, Real{6.1803398874989501e-01}, Real{6.1803398874989501e-01}},
                    Real3{Real{1.9098300562505299e-01}, Real{1.9098300562505299e-01}, Real{1.9098300562505399e-01}},
                    Real3{Real{-4.0901699437494698e-01}, Real{-1.0000000000000001e-01}, Real{-3.3100455376630302e-16}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.9098300562505299e-01}, Real{6.8277561748882798e-16}},
                    Real3{Real{-1.6180339887499000e+00}, Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{2.0000000000000000e+00}, Real{2.0000000000000000e+00}, Real{-7.7872795205668897e-16}},
                    Real3{Real{-9.9999999999999797e-02}, Real{7.0901699437494703e-01}, Real{1.3704142075239901e-16}},
                    Real3{Real{4.9088785086304397e-16}, Real{2.4335327255336499e-17}, Real{4.8732246759304895e-16}},
                    Real3{Real{-1.5616510450613501e-16}, Real{5.7464926516065499e-16}, Real{-2.1804347351261201e-16}},
                    Real3{Real{-4.8685262129553397e-16}, Real{2.8536541236814301e-16}, Real{-1.9098300562505199e-01}},
                    Real3{Real{-2.0701643706960201e-16}, Real{1.9805059448242099e-16}, Real{-2.1914715090297802e-15}},
                    Real3{Real{-7.8296211624252901e-16}, Real{-1.5232914148212401e-15}, Real{2.7000000000000002e+00}},
                    Real3{Real{-5.5608854282362604e-16}, Real{-8.7076185334371596e-16}, Real{1.6180339887498900e+00}},
                    Real3{Real{-6.0909054433954002e-17}, Real{1.2695690478184701e-16}, Real{1.0548275063828800e-16}},
                    Real3{Real{3.6102194811919600e-16}, Real{3.1155171839885000e-16}, Real{-6.1803398874989501e-01}},
                    Real3{Real{5.3829709912634002e-16}, Real{1.7357894073344000e-16}, Real{-1.3090169943749499e+00}},
                    Real3{Real{-2.9833422849746902e-17}, Real{-6.6200910753260505e-17}, Real{5.9999999999999998e-01}}},
    DerivativeBlock{Real3{Real{-5.0000000000000100e-01}, Real{-5.0000000000000000e-01}, Real{-5.0000000000000000e-01}},
                    Real3{Real{1.2102047282546501e-16}, Real{7.6442227287986896e-16}, Real{9.4163750862287500e-17}},
                    Real3{Real{1.5543122344752199e-15}, Real{0.0000000000000000e+00}, Real{-6.7025852201758103e-16}},
                    Real3{Real{4.9999999999999900e-01}, Real{0.0000000000000000e+00}, Real{-6.6200910753260603e-16}},
                    Real3{Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}},
                    Real3{Real{1.2299297924047400e-17}, Real{1.7637171600464699e-15}, Real{1.7245207418742600e-16}},
                    Real3{Real{-1.5450849718747399e+00}, Real{0.0000000000000000e+00}, Real{3.9015749570790200e-16}},
                    Real3{Real{-4.0450849718747399e+00}, Real{-4.0450849718747399e+00}, Real{-4.0450849718747399e+00}},
                    Real3{Real{4.0450849718747399e+00}, Real{-1.4113683131279100e-15}, Real{-5.4870120761681097e-16}},
                    Real3{Real{1.4702669139819701e-16}, Real{3.0000000000000000e+00}, Real{2.2017464522045900e-16}},
                    Real3{Real{-1.9095206750509599e-16}, Real{-5.8484393191044003e-17}, Real{1.8673697732338201e-16}},
                    Real3{Real{-4.0384939064166001e-16}, Real{1.1861907147574200e-16}, Real{8.7217389405044598e-16}},
                    Real3{Real{1.5660917718109999e-16}, Real{4.2505546918519501e-16}, Real{1.1087783439604300e-15}},
                    Real3{Real{-3.6999979096586402e-16}, Real{-2.8473860123848201e-16}, Real{-1.8290014761317500e-15}},
                    Real3{Real{-1.5116753989585300e-15}, Real{-2.1347006689523301e-16}, Real{-9.5485043737666598e-16}},
                    Real3{Real{-1.1121770856472499e-15}, Real{-1.0507849281766900e-15}, Real{4.0450849718747399e+00}},
                    Real3{Real{-1.2181810886790800e-16}, Real{-4.4728161958144100e-16}, Real{-1.6864004925422200e-16}},
                    Real3{Real{7.2204389623839298e-16}, Real{1.0200706312815401e-18}, Real{-1.8270148342763300e-16}},
                    Real3{Real{1.0765941982526800e-15}, Real{-8.6104661904355100e-17}, Real{-1.5450849718747399e+00}},
                    Real3{Real{-5.9666845699493804e-17}, Real{-3.2670325268834501e-18}, Real{5.0000000000000000e-01}}},
    DerivativeBlock{Real3{Real{-7.0901699437494803e-01}, Real{-7.0901699437494803e-01}, Real{-8.0901699437494901e-01}},
                    Real3{Real{1.6180339887498900e+00}, Real{1.1331217120074199e-15}, Real{3.2512517428166301e-16}},
                    Real3{Real{-1.3090169943749499e+00}, Real{3.6869943912755599e-16}, Real{4.1717526624120102e-16}},
                    Real3{Real{5.9999999999999998e-01}, Real{-2.2066970251086900e-16}, Real{-1.0269150607737900e-16}},
                    Real3{Real{3.2945162591403501e-17}, Real{1.6180339887499000e+00}, Real{-8.1584124673710904e-17}},
                    Real3{Real{-8.7202972116012502e-16}, Real{-1.3186468668809400e-15}, Real{2.5054016844745902e-16}},
                    Real3{Real{4.2962002342968699e-16}, Real{-9.7539373926975403e-17}, Real{3.2039385244111598e-16}},
                    Real3{Real{7.8126440576178098e-16}, Real{-1.3090169943749499e+00}, Real{2.8382060050662998e-16}},
                    Real3{Real{2.8480720446978800e-16}, Real{3.1050102888814101e-16}, Real{-1.6291464744890401e-17}},
                    Real3{Real{-4.0290646762322300e-16}, Real{5.9999999999999998e-01}, Real{-1.4018912212719001e-16}},
                    Real3{Real{-2.0000000000000000e+00}, Real{-2.0000000000000000e+00}, Real{1.2237429076471700e-15}},
                    Real3{Real{2.7000000000000002e+00}, Real{-8.7217389405044598e-16}, Real{-9.7329074487427603e-16}},
                    Real3{Real{-6.1803398874989401e-01}, Real{5.7990729458313699e-16}, Real{5.4331124581345997e-16}},
                    Real3{Real{-1.4668544307334700e-15}, Real{2.7000000000000002e+00}, Real{-6.8680160376295196e-16}},
                    Real3{Real{-1.0833469166532601e-15}, Real{-1.0344213071580601e-15}, Real{1.8978699984896099e-16}},
                    Real3{Real{2.9142131463154799e-16}, Real{-6.1803398874989501e-01}, Real{2.9142131463154799e-16}},
                    Real3{Real{-1.9098300562505199e-01}, Real{-1.9098300562505299e-01}, Real{1.1180339887498900e+00}},
                    Real3{Real{-1.9098300562505299e-01}, Real{9.1350741713816301e-17}, Real{-1.9992206108636700e-16}},
                    Real3{Real{3.2816752190129601e-16}, Real{-1.9098300562505199e-01}, Real{-1.8752429822931199e-16}},
                    Real3{Real{9.9999999999999895e-02}, Real{9.9999999999999895e-02}, Real{-3.0901699437494701e-01}}},
    DerivativeBlock{Real3{Real{3.3333333333333298e-01}, Real{3.8888888888888901e-01}, Real{3.3333333333333398e-01}},
                    Real3{Real{-6.2112999374994204e-01}, Real{-7.2723166354163804e-01}, Real{-7.2723166354163704e-01}},
                    Real3{Real{6.2112999374994105e-01}, Real{-1.0610166979169600e-01}, Real{-1.0610166979169600e-01}},
                    Real3{Real{-3.3333333333333298e-01}, Real{5.5555555555555601e-02}, Real{-4.8271497424252499e-17}},
                    Real3{Real{-1.7054765120929501e-16}, Real{-1.0610166979169600e-01}, Real{-6.1310498873456005e-17}},
                    Real3{Real{-2.9067657372004202e-16}, Real{1.5000000000000000e+00}, Real{-1.0900371514501600e-16}},
                    Real3{Real{-2.7968106978839502e-17}, Real{-1.0610166979169600e-01}, Real{-1.1796863878579399e-16}},
                    Real3{Real{1.3021073429363000e-16}, Real{-7.2723166354163704e-01}, Real{5.2084293717452098e-16}},
                    Real3{Real{-1.7685325183658700e-16}, Real{-7.2723166354163704e-01}, Real{3.7421377574435000e-16}},
                    Real3{Real{-5.5959231614336498e-17}, Real{6.6666666666666596e-01}, Real{-1.5668584852014200e-16}},
                    Real3{Real{-7.2723166354163704e-01}, Real{-7.2723166354163704e-01}, Real{-6.2112999374994204e-01}},
                    Real3{Real{1.8448149077006301e-16}, Real{-1.5000000000000000e+00}, Real{7.8535887818221402e-17}},
                    Real3{Real{7.2723166354163704e-01}, Real{0.0000000000000000e+00}, Real{1.0610166979169600e-01}},
                    Real3{Real{-1.6114247091010501e-17}, Real{1.5000000000000000e+00}, Real{-4.3928020816126598e-16}},
                    Real3{Real{9.3459328360478596e-17}, Real{1.5000000000000000e+00}, Real{-9.8590635531003302e-19}},
                    Real3{Real{2.1325975227729901e-17}, Real{-7.2723166354163704e-01}, Real{2.4285109552629002e-16}},
                    Real3{Real{-1.0610166979169600e-01}, Real{-1.0610166979169600e-01}, Real{6.2112999374994105e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{6.8513056285362201e-17}, Real{7.2723166354163704e-01}},
                    Real3{Real{1.3554426140800300e-17}, Real{-1.0610166979169600e-01}, Real{-1.8752429822931199e-16}},
                    Real3{Real{-5.0032178999068501e-17}, Real{5.5555555555555497e-02}, Real{-3.3333333333333298e-01}}},
    DerivativeBlock{Real3{Real{-5.9999999999999998e-01}, Real{-6.0000000000000098e-01}, Real{-5.9999999999999898e-01}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749399e+00}},
                    Real3{Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{7.0901699437494703e-01}, Real{-1.1033485125543401e-16}, Real{-1.0000000000000001e-01}},
                    Real3{Real{1.7738956547843700e-16}, Real{6.6659381028159500e-16}, Real{-6.1000980808580196e-16}},
                    Real3{Real{5.8135314744008295e-16}, Real{-1.6461360160433700e-15}, Real{-1.4533828686002099e-15}},
                    Real3{Real{-2.9038423102594797e-17}, Real{1.6180339887499000e+00}, Real{7.2186583008478400e-16}},
                    Real3{Real{3.9063220288089000e-16}, Real{7.8126440576178098e-16}, Real{1.3021073429363000e-16}},
                    Real3{Real{-7.6703466717672195e-16}, Real{-1.3090169943749499e+00}, Real{-7.5078978773214397e-16}},
                    Real3{Real{2.7979615807168299e-16}, Real{5.9999999999999998e-01}, Real{1.9026138748874400e-16}},
                    Real3{Real{6.1803398874989302e-01}, Real{6.1803398874989401e-01}, Real{6.1803398874989302e-01}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{2.0000000000000000e+00}, Real{-2.6845320276049402e-16}, Real{2.0000000000000000e+00}},
                    Real3{Real{1.4211857431810500e-15}, Real{-1.3148829054178699e-15}, Real{4.2393365123370499e-16}},
                    Real3{Real{1.2568253800038201e-15}, Real{2.7000000000000002e+00}, Real{1.0813751039426400e-15}},
                    Real3{Real{9.7140438210515906e-17}, Real{-6.1803398874989501e-01}, Real{1.9428087642103201e-16}},
                    Real3{Real{1.9098300562505299e-01}, Real{1.9098300562505299e-01}, Real{1.9098300562505299e-01}},
                    Real3{Real{1.9098300562505299e-01}, Real{7.0796824828207598e-16}, Real{1.3090169943749499e+00}},
                    Real3{Real{-9.3762149114655996e-17}, Real{-1.9098300562505199e-01}, Real{1.8752429822931199e-16}},
                    Real3{Real{-9.9999999999999895e-02}, Real{-9.1026252285733095e-17}, Real{-4.0901699437494698e-01}}},
    DerivativeBlock{Real3{Real{3.8888888888888901e-01}, Real{3.3333333333333398e-01}, Real{3.3333333333333398e-01}},
                    Real3{Real{-1.0610166979169600e-01}, Real{-9.7963098136445696e-17}, Real{-1.3931606692034400e-16}},
                    Real3{Real{-7.2723166354163704e-01}, Real{4.7535360652345603e-16}, Real{1.4734228540960599e-16}},
                    Real3{Real{6.6666666666666596e-01}, Real{-1.7929413329008100e-16}, Real{-1.1033485125543401e-16}},
                    Real3{Real{-7.2723166354163704e-01}, Real{-6.2112999374994105e-01}, Real{-7.2723166354163704e-01}},
                    Real3{Real{1.5000000000000000e+00}, Real{-5.6149848134701700e-16}, Real{3.7327305019658502e-16}},
                    Real3{Real{-7.2723166354163704e-01}, Real{1.9507874785395100e-16}, Real{-6.0962108704359603e-18}},
                    Real3{Real{-1.0610166979169600e-01}, Real{6.2112999374994204e-01}, Real{-1.0610166979169600e-01}},
                    Real3{Real{-1.0610166979169600e-01}, Real{1.8347788070662900e-16}, Real{1.5075708188732799e-16}},
                    Real3{Real{5.5555555555555400e-02}, Real{-3.3333333333333298e-01}, Real{-3.3575538968601902e-17}},
                    Real3{Real{-7.2723166354163604e-01}, Real{-7.2723166354163704e-01}, Real{-6.2112999374994105e-01}},
                    Real3{Real{1.5000000000000000e+00}, Real{-4.3608694702522299e-16}, Real{0.0000000000000000e+00}},
                    Real3{Real{-7.2723166354163704e-01}, Real{2.0945146911001399e-16}, Real{1.8252850507817900e-16}},
                    Real3{Real{-1.5000000000000000e+00}, Real{-1.3470116883034199e-16}, Real{4.5249375944965799e-16}},
                    Real3{Real{1.5000000000000000e+00}, Real{1.5914173956277800e-16}, Real{4.8737157741100701e-16}},
                    Real3{Real{7.2855328657886899e-17}, Real{7.2723166354163704e-01}, Real{1.0610166979169600e-01}},
                    Real3{Real{-1.0610166979169600e-01}, Real{-1.0610166979169600e-01}, Real{6.2112999374994105e-01}},
                    Real3{Real{-1.0610166979169600e-01}, Real{-2.2837685428454100e-17}, Real{0.0000000000000000e+00}},
                    Real3{Real{2.1096483550797601e-16}, Real{1.0610166979169600e-01}, Real{7.2723166354163704e-01}},
                    Real3{Real{5.5555555555555497e-02}, Real{4.5513126142866597e-17}, Real{-3.3333333333333298e-01}}},
    DerivativeBlock{Real3{Real{-6.6666666666666696e-01}, Real{-6.6666666666666696e-01}, Real{-6.6666666666666596e-01}},
                    Real3{Real{7.2723166354163704e-01}, Real{7.2723166354163704e-01}, Real{7.2723166354163704e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}},
                    Real3{Real{-3.8888888888888901e-01}, Real{-5.5555555555555497e-02}, Real{-5.5555555555555400e-02}},
                    Real3{Real{7.2723166354163604e-01}, Real{7.2723166354163704e-01}, Real{7.2723166354163704e-01}},
                    Real3{Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}},
                    Real3{Real{7.2723166354163704e-01}, Real{1.0610166979169600e-01}, Real{3.5967644135572201e-16}},
                    Real3{Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{7.2723166354163704e-01}, Real{-4.0576839002427500e-16}},
                    Real3{Real{-5.5555555555555400e-02}, Real{-3.8888888888888801e-01}, Real{-5.5555555555555497e-02}},
                    Real3{Real{7.2723166354163704e-01}, Real{7.2723166354163704e-01}, Real{7.2723166354163604e-01}},
                    Real3{Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}},
                    Real3{Real{7.2723166354163704e-01}, Real{-1.9330243152771201e-16}, Real{1.0610166979169500e-01}},
                    Real3{Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}, Real{-1.5000000000000000e+00}},
                    Real3{Real{1.5000000000000000e+00}, Real{1.5000000000000000e+00}, Real{1.5000000000000000e+00}},
                    Real3{Real{-8.4997883434201402e-17}, Real{7.2723166354163704e-01}, Real{1.0610166979169600e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}, Real{1.0610166979169600e-01}},
                    Real3{Real{1.0610166979169600e-01}, Real{-4.5675370856908101e-17}, Real{7.2723166354163704e-01}},
                    Real3{Real{-9.3762149114655996e-17}, Real{1.0610166979169600e-01}, Real{7.2723166354163704e-01}},
                    Real3{Real{-5.5555555555555497e-02}, Real{-5.5555555555555497e-02}, Real{-3.8888888888888901e-01}}},
    DerivativeBlock{Real3{Real{-5.9999999999999998e-01}, Real{-6.0000000000000098e-01}, Real{-5.9999999999999998e-01}},
                    Real3{Real{3.8644922436892399e-16}, Real{9.3672559929298691e-16}, Real{7.9403160804454803e-16}},
                    Real3{Real{1.0362081563168101e-15}, Real{-5.9211894646675002e-16}, Real{-3.5260180127525501e-16}},
                    Real3{Real{5.9999999999999898e-01}, Real{0.0000000000000000e+00}, Real{-1.6550227688315200e-16}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}},
                    Real3{Real{2.9317943373009499e-16}, Real{1.2868400114586701e-15}, Real{1.1778362963136499e-15}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-1.9507874785395100e-16}, Real{1.9507874785395100e-16}},
                    Real3{Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{1.6180339887498900e+00}, Real{-5.2220627585732796e-16}, Real{-5.6807574603398497e-16}},
                    Real3{Real{-6.7151077937203903e-17}, Real{7.0901699437494703e-01}, Real{-9.9999999999999895e-02}},
                    Real3{Real{6.1803398874989401e-01}, Real{6.1803398874989501e-01}, Real{6.1803398874989501e-01}},
                    Real3{Real{-6.1107851829159099e-16}, Real{1.7443477881008900e-15}, Real{1.7443477881008900e-15}},
                    Real3{Real{-6.1803398874989401e-01}, Real{-1.9330243152771201e-16}, Real{3.8660486305542501e-16}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{2.7000000000000002e+00}, Real{-3.1828347912555501e-16}, Real{1.5914173956277800e-16}},
                    Real3{Real{1.9730792124268499e-16}, Real{2.0000000000000000e+00}, Real{2.0000000000000000e+00}},
                    Real3{Real{1.9098300562505299e-01}, Real{1.9098300562505199e-01}, Real{1.9098300562505199e-01}},
                    Real3{Real{-1.9098300562505199e-01}, Real{1.5986379799917901e-16}, Real{-3.6540296685526501e-16}},
                    Real3{Real{-4.9402093207666600e-17}, Real{1.9098300562505199e-01}, Real{1.3090169943749499e+00}},
                    Real3{Real{-7.5239016465664398e-17}, Real{-9.9999999999999895e-02}, Real{-4.0901699437494698e-01}}},
    DerivativeBlock{Real3{Real{4.0901699437494798e-01}, Real{4.0901699437494798e-01}, Real{3.0901699437494801e-01}},
                    Real3{Real{-1.9098300562505299e-01}, Real{-1.0352560810353000e-15}, Real{-3.6190146217476301e-16}},
                    Real3{Real{-6.1803398874989401e-01}, Real{-3.1951847880474598e-17}, Real{2.6320291169285800e-17}},
                    Real3{Real{5.9999999999999998e-01}, Real{6.8959282034646499e-17}, Real{-1.5111982935196099e-18}},
                    Real3{Real{-4.4139780516897498e-16}, Real{-1.9098300562505199e-01}, Real{-1.3894926241878801e-16}},
                    Real3{Real{-2.9067657372004202e-16}, Real{-2.1204570281516700e-17}, Real{-4.3601486058006202e-16}},
                    Real3{Real{2.9261812178092602e-16}, Real{3.4138780874441399e-16}, Real{-1.8539927348641899e-16}},
                    Real3{Real{0.0000000000000000e+00}, Real{-6.1803398874989501e-01}, Real{-1.3021073429363000e-16}},
                    Real3{Real{5.0463489116126398e-16}, Real{1.4113683131279101e-17}, Real{-2.0358280724689801e-16}},
                    Real3{Real{-4.4767385291469198e-16}, Real{5.9999999999999998e-01}, Real{-4.4767385291469197e-17}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-1.3090169943749499e+00}, Real{-1.1180339887498900e+00}},
                    Real3{Real{2.7000000000000002e+00}, Real{-8.7217389405044598e-16}, Real{-9.0128390131488597e-17}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-1.9330243152771201e-16}, Real{-4.8824363702964798e-17}},
                    Real3{Real{-4.9341062552540902e-16}, Real{2.7000000000000002e+00}, Real{3.2021464414031100e-16}},
                    Real3{Real{-1.3699992291373200e-15}, Real{-3.1828347912555501e-16}, Real{5.2455263897017402e-17}},
                    Real3{Real{3.3999153373680600e-16}, Real{-1.3090169943749499e+00}, Real{8.0545990907591798e-17}},
                    Real3{Real{-2.0000000000000000e+00}, Real{-2.0000000000000000e+00}, Real{5.7587025691724703e-16}},
                    Real3{Real{1.6180339887498900e+00}, Real{5.4810445028289800e-16}, Real{-1.9340963702042001e-16}},
                    Real3{Real{-2.3440537278664000e-16}, Real{1.6180339887498900e+00}, Real{3.7504859645862399e-16}},
                    Real3{Real{9.9999999999999797e-02}, Real{9.9999999999999506e-02}, Real{8.0901699437494601e-01}}},
    DerivativeBlock{Real3{Real{-6.0000000000000098e-01}, Real{-6.0000000000000098e-01}, Real{-5.9999999999999998e-01}},
                    Real3{Real{6.1803398874989501e-01}, Real{6.1803398874989601e-01}, Real{6.1803398874989501e-01}},
                    Real3{Real{1.9098300562505199e-01}, Real{1.9098300562505199e-01}, Real{1.9098300562505199e-01}},
                    Real3{Real{-4.0901699437494698e-01}, Real{3.8617197939401999e-16}, Real{-9.9999999999999298e-02}},
                    Real3{Real{5.4747878803662795e-16}, Real{1.2605006735221400e-15}, Real{3.4214978099263599e-16}},
                    Real3{Real{-8.7202972116012403e-16}, Real{1.3570062948786300e-15}, Real{-5.8135314744008295e-16}},
                    Real3{Real{-3.9015749570790200e-16}, Real{-1.9098300562505199e-01}, Real{-3.9462527721785802e-17}},
                    Real3{Real{0.0000000000000000e+00}, Real{-7.8126440576178098e-16}, Real{-7.8126440576178098e-16}},
                    Real3{Real{-4.3366412997396200e-16}, Real{-6.1803398874989601e-01}, Real{-4.4990900941854002e-16}},
                    Real3{Real{1.3430215587440800e-16}, Real{5.9999999999999898e-01}, Real{2.2383692645734599e-16}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{1.3090169943749499e+00}, Real{-1.1347643320886800e-15}, Real{1.9098300562505099e-01}},
                    Real3{Real{-9.4893499924480495e-17}, Real{4.3829430180595602e-16}, Real{1.3726173011705900e-15}},
                    Real3{Real{-9.4893499924480606e-17}, Real{2.7000000000000002e+00}, Real{-3.8055990605323199e-16}},
                    Real3{Real{-1.4571065731577399e-16}, Real{-1.3090169943749499e+00}, Real{-3.3999153373680600e-16}},
                    Real3{Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{2.0000000000000000e+00}, Real{0.0000000000000000e+00}, Real{2.0000000000000000e+00}},
                    Real3{Real{-9.3762149114655996e-17}, Real{1.6180339887498900e+00}, Real{3.7504859645862399e-16}},
                    Real3{Real{-1.0000000000000001e-01}, Real{-3.9720546451956298e-16}, Real{7.0901699437494603e-01}}},
    DerivativeBlock{Real3{Real{-6.0000000000000098e-01}, Real{-6.0000000000000098e-01}, Real{-6.0000000000000098e-01}},
                    Real3{Real{1.4336000766649001e-15}, Real{9.5543296242633803e-16}, Real{3.2743263728953600e-16}},
                    Real3{Real{-5.9211894646675002e-16}, Real{-7.6451715155336599e-16}, Real{-6.2809520381030000e-16}},
                    Real3{Real{5.9999999999999998e-01}, Real{1.7239820508661599e-16}, Real{1.6550227688315200e-16}},
                    Real3{Real{6.1803398874989501e-01}, Real{6.1803398874989401e-01}, Real{6.1803398874989401e-01}},
                    Real3{Real{-3.9920228513767801e-16}, Real{-3.7394800229080898e-16}, Real{-5.5562086086583597e-16}},
                    Real3{Real{-6.1803398874989501e-01}, Real{-7.6812256967493101e-16}, Real{-2.1946359133569501e-16}},
                    Real3{Real{1.9098300562505199e-01}, Real{1.9098300562505299e-01}, Real{1.9098300562505199e-01}},
                    Real3{Real{-1.9098300562505199e-01}, Real{3.3167155358505899e-16}, Real{-2.9991576653968099e-16}},
                    Real3{Real{-8.9534770582938505e-17}, Real{-4.0901699437494698e-01}, Real{-9.9999999999999895e-02}},
                    Real3{Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}, Real{1.3090169943749499e+00}},
                    Real3{Real{2.6980407361011398e-16}, Real{8.7217389405044598e-16}, Real{8.7217389405044598e-16}},
                    Real3{Real{-1.3090169943749499e+00}, Real{-5.7990729458313699e-16}, Real{-7.7320972611085001e-16}},
                    Real3{Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}, Real{-2.7000000000000002e+00}},
                    Real3{Real{2.7000000000000002e+00}, Real{0.0000000000000000e+00}, Real{-1.7903445700812499e-16}},
                    Real3{Real{4.4800875891200499e-16}, Real{1.3090169943749499e+00}, Real{1.9098300562505199e-01}},
                    Real3{Real{-1.6180339887499000e+00}, Real{-1.6180339887499000e+00}, Real{-1.6180339887498900e+00}},
                    Real3{Real{1.6180339887499000e+00}, Real{-3.6540296685526501e-16}, Real{3.6540296685526501e-16}},
                    Real3{Real{-1.2430783491175601e-16}, Real{2.0000000000000000e+00}, Real{2.0000000000000000e+00}},
                    Real3{Real{-1.4220291908717100e-16}, Real{-1.0000000000000001e-01}, Real{7.0901699437494703e-01}}},
    DerivativeBlock{Real3{Real{-5.0000000000000100e-01}, Real{-5.0000000000000100e-01}, Real{-5.0000000000000100e-01}},
                    Real3{Real{5.1433081888359199e-16}, Real{1.4231625976716099e-15}, Real{7.3658958664189898e-16}},
                    Real3{Real{-6.6613381477509402e-16}, Real{-1.0505725329026100e-16}, Real{-1.8414927012120100e-15}},
                    Real3{Real{5.0000000000000000e-01}, Real{5.8615389729449499e-16}, Real{3.2834759721849600e-17}},
                    Real3{Real{-5.0749368267349301e-16}, Real{2.2805320781159502e-16}, Real{-2.2177534256180299e-16}},
                    Real3{Real{-5.0868400401007204e-16}, Real{5.8135314744008404e-16}, Real{-3.4881188846405001e-15}},
                    Real3{Real{-3.1020875339430799e-16}, Real{-4.0207157973445101e-17}, Real{-1.5172585057406299e-15}},
                    Real3{Real{-9.1147514005541103e-16}, Real{-2.0833717486980800e-15}, Real{-9.1147514005541103e-16}},
                    Real3{Real{2.2708570102256898e-16}, Real{-1.3549135806028001e-15}, Real{-1.6037733246186399e-16}},
                    Real3{Real{-2.0145323381161199e-16}, Real{5.0000000000000000e-01}, Real{6.7151077937203903e-17}},
                    Real3{Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}, Real{1.5450849718747399e+00}},
                    Real3{Real{1.4721620161449999e-15}, Real{1.0338067162700699e-15}, Real{3.8616384275892899e-16}},
                    Real3{Real{-1.5450849718747399e+00}, Real{-1.3977884078995201e-15}, Real{-8.1775215913001395e-16}},
                    Real3{Real{3.0138862162076798e-15}, Real{4.7290846606975501e-15}, Real{1.9801711648134799e-15}},
                    Real3{Real{-2.2456454054637900e-15}, Real{1.5914173956277801e-15}, Real{-1.5261832496341700e-15}},
                    Real3{Real{4.6161108564519496e-16}, Real{-1.5450849718747399e+00}, Real{-3.8856175284206298e-16}},
                    Real3{Real{-4.0450849718747399e+00}, Real{-4.0450849718747399e+00}, Real{-4.0450849718747399e+00}},
                    Real3{Real{4.0450849718747399e+00}, Real{-7.3080593371053001e-16}, Real{-8.1980319421046397e-16}},
                    Real3{Real{-6.0352647705985398e-16}, Real{4.0450849718747399e+00}, Real{0.0000000000000000e+00}},
                    Real3{Real{-2.9093990322810902e-16}, Real{-1.1916163935586901e-15}, Real{3.0000000000000000e+00}}}};


#endif /* HELP_HPP */
