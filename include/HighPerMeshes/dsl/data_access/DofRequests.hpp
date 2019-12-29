// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_DOFREQUESTS_HPP
#define DSL_DATAACCESS_DOFREQUESTS_HPP

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>

//! \name
//! DofRequests
//!
//! DofRequests are defined in access definitions to specify which degrees of freedom are wanted in a local buffer.
//!
//! \{
namespace HPM::dof_requests
{
    namespace internal
    {
        //! Initialize a Constexpr given a specific request
        template <std::size_t Dimension, typename Request>
        constexpr auto Init()
        {
            return auxiliary::ConstexprFor<Dimension>([](const auto I) { return Request{}(I); });
        }

        //! Requests for a dimension passed as value
        //! \{
        template <std::size_t CompareTo>
        struct Equals
        {
            constexpr auto operator()(const std::size_t value) { return (value == CompareTo); };
        };

        struct All
        {
            constexpr auto operator()(const std::size_t) { return true; };
        };
        //! \}

    } // namespace internal

    //! Given a dimensionality `Dimension` and a dimension `CompareTo`, `RequestEquals` constructs a ConstexprArray of size `Dimension`
    //! that specifies a specific wanted dimension.
    template <std::size_t Dimension, std::size_t CompareTo>
    constexpr auto RequestEquals = internal::Init<Dimension, internal::Equals<CompareTo>>();

    //! Given a dimensionality `Dimension` and a dimension `CompareTo`, `RequestAll` constructs a ConstexprArray of size `Dimension`
    //! that specifies that all dimensions are wanted.
    template <std::size_t Dimension>
    constexpr auto RequestAll = internal::Init<Dimension, internal::All>();
} // namespace HPM::dofRequests
//! \}

#endif