// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_ACCESSPATTERNS_HPP
#define DSL_DATAACCESS_ACCESSPATTERNS_HPP

namespace HPM::AccessPatterns
{
    //! \name
    //! AccessPatterns
    //!
    //! These patterns describe a topological relationship between some entity in the mesh with some other entity in the mesh.
    //! By providing them to access definitions in a loop the run time system is able to provide the correct local views to the kernels.
    //!
    //! \{
    auto SimplePattern = [](auto&& entity) { return entity; };

    auto NeighboringMeshElementOrSelfPattern = [](auto&& entity) { return entity.GetTopology().GetNeighboringCell(); };

    auto ContainingMeshElementPattern = [](auto&& entity) { return entity.GetTopology().GetContainingCell(); };
    //! \}
} // namespace HPM::AccessPatterns

#endif