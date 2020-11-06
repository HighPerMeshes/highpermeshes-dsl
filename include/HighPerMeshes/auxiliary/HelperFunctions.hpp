#ifndef HELPERFUNCTIONS_HPP
#define HELPERFUNCTIONS_HPP

#include <vector>
#include <tuple>
#include <HighPerMeshes.hpp>

template<typename Range, typename Op> auto MakeBuffer(const Range& range, Op op) {
    using Type = decltype(op(*(range.GetEntities().begin())));
    std::vector<Type> res;

    for(const auto& entity : range.GetEntities()) {
        res.emplace_back(op(entity));
    } 

    return res; 
}

template<typename Range>
auto GetInverseJacobian(const Range& range) {
    return MakeBuffer(range, [](const auto& e) { return e.GetGeometry().GetInverseJacobian(); });
}

template<typename MeshLoop>
auto GetBuffers(MeshLoop& mesh_loop) {
    return std::apply(
        [](auto&... access_definitions) { return std::tie(*access_definitions.buffer...); }
        , mesh_loop.access_definitions
    );
}

template<typename AccessDefinition>
auto GetOffset(AccessDefinition& access_definition) {
    return access_definition.buffer->GetOffsets()[access_definition.RequestedDimension];
}

template<typename MeshLoop>
auto GetOffsets(MeshLoop& mesh_loop) {
    return std::apply(
        [](auto&... access_definitions) {
            return std::tuple { 
                GetOffset(access_definitions)...
            };
        },
        mesh_loop.access_definitions
    );
}

template<typename Mesh>
auto MakeMeshInfo(HPM::OpenCLHandler& handler, const Mesh& mesh) {
    
    std::vector<size_t, cl::SVMAllocator<size_t, cl::SVMTraitCoarse<>>> mesh_info(mesh.CellDimension + 1, handler.GetSVMAllocator<int>());
    HPM::auxiliary::ConstexprFor<mesh.CellDimension + 1>(
        [&](auto index) {
            static constexpr auto i = index.value;
            mesh_info[i] = mesh.template GetNumEntities<i>();
        }
    );
    return mesh_info;
}

#endif /* HELPERFUNCTIONS_HPP */
