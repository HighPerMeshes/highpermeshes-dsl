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
        DofPartition(DataT& data, const std::size_t offset, const std::size_t size, const std::size_t dimension) : data(data), offset(offset), size(size), dimension(dimension) {}

        auto begin() { return data.begin() + offset; }

        auto begin() const { return data.begin() + offset; }

        auto end() { return data.begin() + offset + size; }

        auto end() const { return data.begin() + offset + size; }

        auto GetSize() const { return size; }

        auto GetDimension() const { return dimension; }
        
      protected:
        DataT& data;
        const std::size_t offset;
        const std::size_t size;
        const std::size_t dimension;
    };
} // namespace HPM

#endif