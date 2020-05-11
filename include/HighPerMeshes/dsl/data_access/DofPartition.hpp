// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_DOFPARTITION_HPP
#define DSL_DATAACCESS_DOFPARTITION_HPP

#include <cstdint>

namespace HPM
{
    template <typename DataT>
    class DofPartition
    {
      public:
        DofPartition(DataT& data, const std::size_t offset, const std::size_t size, const std::size_t dofs_per_entity, const std::size_t dimension) : data(data), offset(offset), size(size), dofs_per_entity(dofs_per_entity), dimension(dimension) {}

        auto begin() { return data.begin() + offset; }

        auto begin() const { return data.begin() + offset; }

        auto end() { return data.begin() + offset + size; }

        auto end() const { return data.begin() + offset + size; }

        auto& operator[](const std::size_t index)
        { 
            assert(index < size);

            return data[offset + index];
        }

        const auto& operator[](const std::size_t index) const
        { 
            assert(index < size);

            return data[offset + index];
        }

        auto GetSize() const { return size; }

        auto GetDimension() const { return dimension; }

        //! Resolve dof start position inside this partition for a given entity.
        //!
        //! \return DofPartition of the specified entity
        template <typename EntityT>
        auto At(const EntityT& entity) const -> DofPartition<DataT>
        {
            assert(EntityT::Dimension <= dimension);
            
            // Start position.
            const std::size_t index = entity.GetTopology().GetIndex() * dofs_per_entity;

            assert((index + dofs_per_entity) < size);

            // Return a DofPartition that contains only the dofs of the specified entity.
            return {data, offset + index, dofs_per_entity, dofs_per_entity, dimension};
        }
        
      protected:
        DataT& data;
        const std::size_t offset;
        const std::size_t size;
        const std::size_t dofs_per_entity;
        const std::size_t dimension;
    };
} // namespace HPM

#endif