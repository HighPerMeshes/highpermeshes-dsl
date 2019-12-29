// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef MISC_DG_HPP
#define MISC_DG_HPP

#include <HighPerMeshes/dsl/meshes/Mesh.hpp>

namespace HPM::DG
{
    template <std::size_t NumSurfaceNodes>
    using SurfaceMap = std::array<std::array<std::size_t, 2>, NumSurfaceNodes>;

    template <typename BufferT_1, typename BufferT_2, typename T>
    auto Delta(const BufferT_1& buffer, const BufferT_2& neighboring_buffer, const int index, const T& face_node_to_cell_mapping)
    {
        return neighboring_buffer[face_node_to_cell_mapping[index][1]] - buffer[face_node_to_cell_mapping[index][0]];
    }

    template <typename FaceT>
    auto Direction(const FaceT& face) -> ::HPM::dataType::Real
    {
        return (face.GetTopology().HasNeighboringCell() ? 1.0 : -1.0);
    }

    template <typename BufferT_1, typename BufferT_2, typename FaceT, typename T>
    auto DirectionalDelta(const BufferT_1& buffer, const BufferT_2& neighboring_buffer, const FaceT& face, const int index, const T& face_node_to_cell_mapping)
    {
        return Direction(face) * neighboring_buffer[face_node_to_cell_mapping[index][1]] - buffer[face_node_to_cell_mapping[index][0]];
    }

    // \todo { Not sure if threshold is the right name - Stefan G. 23.07.2019 }
    template <typename DgInfo, typename EntityT, typename FaceT>
    static auto ComputeForOneFace(const EntityT& element, const FaceT& face, double threshold = 1.0E-4)
    {
        const std::size_t face_index = face.GetTopology().GetLocalIndex();
        const auto& element_nodes = element.GetTopology().GetNodes();
        const auto& neighboring_element_nodes = face.GetTopology().GetNeighboringCell().GetTopology().GetNodes();
        const std::size_t neighboring_face_index = face.GetTopology().GetLocalIndexOfNeighboringFace();
        SurfaceMap<DgInfo::NumSurfaceNodes> result;

        for (std::size_t n1 = 0; n1 < DgInfo::NumSurfaceNodes; ++n1)
        {
            result[n1][0] = DgInfo::localMask[face_index][n1];
            
            // Go through all degrees of freedom for a surface of the neighboring face and find the one that has a distance below a certain threshhold. If it can't be found it throws an exception.
            for (std::size_t n2 = 0; n2 < DgInfo::NumSurfaceNodes; ++n2)
            {
                // Find normalized distance between these nodes and check if it is (almost) zero
                const auto& d = DgInfo::LocalToGlobal(DgInfo::referenceCoords[DgInfo::localMask[face_index][n1]], element_nodes) -
                                DgInfo::LocalToGlobal(DgInfo::referenceCoords[DgInfo::localMask[neighboring_face_index][n2]], neighboring_element_nodes);

                if (d.Norm() < threshold)
                {
                    result[n1][1] = DgInfo::localMask[neighboring_face_index][n2];
                    break;
                }

                if (n2 == DgInfo::NumSurfaceNodes - 1)
                {
                    throw std::runtime_error("Couldn't find matching neighbor node!");
                }
            }
        }

        return result;
    }

    template <typename DgInfo, typename MeshT>
    struct DgNodesMap
    {
        // \todo { 4 seems like a magic number to me and should probably be inferred by the Mesh topology - Stefan G. 23.07.2019 }
        // \todo { What is the meaning behind this map for the last index? Map[element index][local face index][DOF of face][??] - Stefan G. 23.07.2019 }
        using Map = std::vector<std::array<SurfaceMap<DgInfo::NumSurfaceNodes>, 4>>;

        DgNodesMap(const MeshT& mesh) : mesh(mesh), map(mesh.GetNumEntities())
        {
            for (auto const& element : mesh.GetEntities())
            {
                const std::size_t element_index = element.GetTopology().GetIndex();
                for (auto const& face : element.GetTopology().GetSubEntities())
                {
                    const std::size_t face_index = face.GetTopology().GetLocalIndex();
                    map[element_index][face_index] = ComputeForOneFace<DgInfo>(element, face);
                }
            }
        }

        template <typename FaceT>
        auto Get(const typename MeshT::CellT& element, const FaceT& face) const -> const SurfaceMap<DgInfo::NumSurfaceNodes>&
        {
            static_assert(std::is_same_v<typename FaceT::ParentEntityT, typename MeshT::CellT>, "face must be of face type in the given mesh");

            return map[element.GetTopology().GetIndex()][face.GetTopology().GetLocalIndex()];
        }

        const MeshT& mesh;
        Map map;
    };
} // namespace HPM::DG

#endif