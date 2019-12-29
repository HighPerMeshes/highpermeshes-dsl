// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_DOF_HPP
#define DSL_DATAACCESS_DOF_HPP

#include <cstdint>
#include <tuple>

#include <HighPerMeshes/auxiliary/ConstexprFor.hpp>
#include <HighPerMeshes/auxiliary/ConstexprIfElse.hpp>
#include <HighPerMeshes/common/Method.hpp>
#include <HighPerMeshes/dsl/buffers/LocalBuffer.hpp>

namespace HPM::dof
{
    using ::HPM::auxiliary::ConstexprFor;
    using ::HPM::auxiliary::ConstexprIfElse;
    using ::HPM::dataType::ConstArray;
    using ::HPM::dataType::ConstexprArray;
    using ::HPM::internal::InvalidLocalBuffer;

    //!
    //! \brief Create a dof array (compile-time constant).
    //!
    //! \tparam Dofs a variadic list of the dofs
    //! \return a `ConstexprArray` with the dofs as non-type template parameters
    //!
    template <std::size_t... Dofs>
    constexpr auto MakeDofs()
    {
        return ConstexprArray<std::size_t, Dofs...>{};
    }

    //!
    //! \brief Create a dof array (runtime).
    //!
    //! \tparam Dofs the type of the dofs
    //! \param dofs a variadic list of the dofs values
    //! \return a `ConstArray` containing the dofs
    //!
    template <typename... Dofs>
    constexpr auto MakeDofs(const Dofs... dofs)
    {
        return ConstArray<std::size_t, sizeof...(Dofs)>(dofs...);
    }

    //!
    //! \brief A helper function for the dof creation.
    //!
    //! \tparam Method the integration method to be used
    //! \tparam Dimension the dimensionality of the cell entity
    //! \param order the integration order
    //! \return a `ConstArray` containing the dofs
    //!
    template <::HPM::method::Name Method, std::size_t Dimension>
    constexpr auto MakeDofs(const std::size_t order);

    //!
    //! \brief Dof creation for DG method in 3 dimensions (specialization).
    //!
    template <>
    constexpr auto MakeDofs<::HPM::method::Name::DG, 3>(const std::size_t order)
    {
        if (order == 3)
        {
            return ConstArray<std::size_t, 4>(0, 0, 0, 20);
        }

        // throw std::runtime_error("error: not implemented");

        return ConstArray<std::size_t, 4>(0, 0, 0, 0);
    }

    //!
    //! \brief Dof creation for LagrangeFEM method in 3 dimensions (specialization).
    //!
    template <>
    constexpr auto MakeDofs<::HPM::method::Name::LagrangeFEM, 3>(const std::size_t order)
    {
        if (order == 1)
        {
            return ConstArray<std::size_t, 4>(1, 0, 0, 0);
        }
        else if (order == 2)
        {
            return ConstArray<std::size_t, 4>(1, 1, 0, 0);
        }
        else if (order == 3)
        {
            return ConstArray<std::size_t, 4>(1, 2, 1, 0);
        }
        else if (order == 4)
        {
            return ConstArray<std::size_t, 4>(1, 3, 3, 1);
        }
        else if (order == 5)
        {
            return ConstArray<std::size_t, 4>(1, 4, 6, 4);
        }
        else if (order == 6)
        {
            return ConstArray<std::size_t, 4>(1, 5, 10, 10);
        }

        // throw std::runtime_error("error: not implemented");

        return ConstArray<std::size_t, 4>(0, 0, 0, 0);
    }

    //!
    //! \brief Names of entity types.
    //!
    //! `Face` and ` Cell` have special values in order to detect them during the processing,
    //! and to map them to the right one depending on the actual value of the cell dimension.
    //!
    struct Name
    {
        enum
        {
            Node = 0,
            Edge = 1,
            Face = 10000,
            Cell = 10001
        };
    };

    namespace
    {
        //!
        //! \brief Resolve the dimension of an entity.
        //!
        //! If the entity is `Face` or `Cell` the actual dimension is `CellDimension-1` and `CellDimension`, respectively.
        //!
        //! \tparam PseudoDimension the dimension according to the entity names above
        //! \tparam CellDimension the dimension of the cell entity
        //! \return the actual dimension
        //!
        template <std::size_t PseudoDimension, std::size_t CellDimension>
        constexpr auto ResolveDimension()
        {
            static_assert(CellDimension > 0, "error: this is a node set. Is this a valid mesh?");

            return ConstexprIfElse<PseudoDimension == Name::Cell>(CellDimension, ConstexprIfElse<PseudoDimension == Name::Face>(CellDimension - 1, PseudoDimension));
        }
    } // namespace

    //!
    //! \brief Get the dofs of a certain dimension for a given `LocalView` element.
    //!
    //! The local view element is a tuple of arrays of `LocalBuffers`.
    //! This function provides access to the arrays.
    //! For the `CellDimension`, a reference to the `LocalBuffer` is returned.
    //!
    //! \tparam PseudoDimension the requested dimension (according to the entity names above)
    //! \tparam LocalViewElement a tuple of arrays of `LocalBuffers`
    //! \param local_view a tuple instance
    //! \return a reference to the array of `LocalBuffers` according to the requested dimension: for the `CellDimension`, a reference to the `LocalBuffer` is returned
    //!
    template <std::size_t PseudoDimension, typename LocalViewElement>
    constexpr auto& GetDofs(LocalViewElement& local_view)
    {
        static_assert(std::tuple_size_v<LocalViewElement>> 0, "error: you are trying to process an empty tuple");

        // Get the cell dimension and use it to resolve the `PseudoDimension`.
        constexpr std::size_t TupleSize = std::tuple_size_v<LocalViewElement> - 1;
        constexpr std::size_t CellDimension = std::tuple_element_t<TupleSize, LocalViewElement>{};
        constexpr std::size_t Dimension = ResolveDimension<PseudoDimension, CellDimension>();

        // Check if the `Dimension` is valid and if the requested dimension is for an `InvalidLocalBuffer`.
        static_assert(Dimension <= CellDimension, "error: Dimension is larger than the mesh-element dimension");
        static_assert(!std::is_same_v<InvalidLocalBuffer, typename std::tuple_element_t<Dimension, LocalViewElement>>, "error: you are trying to access an invalid local buffer");

        // If the cell dimension is requested, return a reference to the `LocalBuffer`.
        if constexpr (Dimension == CellDimension)
        {
            return std::get<Dimension>(local_view)[0];
        }
        else
        {
            return std::get<Dimension>(local_view);
        }
    }

    //!
    //! \brief Get the offset into a partition of dofs of a certain `Dimension` relative to some base pointer.
    //!
    //! \tparam PseudoDimension the requested dimension (according to the entity names above)
    //! \tparam DofT the type of the dofs
    //! \tparam MeshT the mesh type
    //! \param mesh a mesh reference
    //! \param dofs an instance of the dof type
    //! \param index an index of an entity to be considered for the offset calculation
    //! \return the offset relative to some base pointer
    //!
    template <std::size_t PseudoDimension, typename DofT, typename MeshT>
    constexpr auto GetOffset(const MeshT& mesh, const DofT& dofs, const std::size_t index = 0)
    {
        // Get the cell dimension and use it to resolve the `PseudoDimension`: the cell dimension is at least 1 (per construction).
        constexpr std::size_t CellDimension = MeshT::CellDimension;
        constexpr std::size_t Dimension = ResolveDimension<PseudoDimension, CellDimension>();

        // Check if the `Dimension` is valid.
        static_assert(Dimension <= CellDimension, "error: Dimension is larger than the mesh-element dimension");
        static_assert(DofT::Size() >= CellDimension, "error: potential out of bounds data access");

        // calculate the shift value inside the partition of dofs of the requested `Dimension`.
        const std::size_t shift = index * dofs.template At<Dimension>();

        // If the requested `Dimension` is the cell dimension, return immediately.
        if constexpr (Dimension == CellDimension)
        {
            return shift;
        }
        // Otherwise, add the size of the dof partitions with dimension `(Dimension+1)..CellDimension`
        else if constexpr (Dimension == (CellDimension - 1))
        {
            return shift + dofs.template At<CellDimension>() * mesh.template GetNumEntities<CellDimension>();
        }
        else
        {
            // These bounds are in terms of codimension.
            constexpr std::size_t Begin = 0;
            constexpr std::size_t End = CellDimension - Dimension;

            static_assert(Begin <= End);
            static_assert(End < DofT::Size(), "error: out of bounds");

            // Prefix sum calculation: result is in 'offset'.
            std::size_t offset = shift;

            ConstexprFor<Begin, End>([&mesh, &offset, &dofs](const auto I) {
                constexpr std::size_t Index = (CellDimension - I);
                offset += dofs.template At<Index>() * mesh.template GetNumEntities<Index>();
            });

            return offset;
        }
    }

    //!
    //! \brief Get the offset into a partition of dofs of a certain `Dimension` relative to some base pointer.
    //!
    //! This function wraps the above function for `ConstexprArrays`.
    //!
    //! \tparam PseudoDimension the requested dimension (according to the entity names above)
    //! \tparam DofT the type of the dofs
    //! \tparam MeshT the mesh type
    //! \param mesh a mesh reference
    //! \param index an index of an entity to be considered for the offset calculation
    //! \return the offset relative to some base pointer
    //!
    template <std::size_t PseudoDimension, typename DofT, typename MeshT>
    constexpr auto GetOffset(const MeshT& mesh, const std::size_t index = 0)
    {
        static_assert(DofT::IsConstexprArray, "error: Dof type instantiation is corrupted");

        // Instanciate the dof type.
        return GetOffset<PseudoDimension>(mesh, DofT{}, index);
    }
} // namespace HPM::dof

#endif