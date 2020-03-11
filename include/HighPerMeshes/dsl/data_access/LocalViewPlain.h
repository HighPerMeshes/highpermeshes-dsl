// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef DSL_DATAACCESS_LOCALVIEWPLAIN_H
#define DSL_DATAACCESS_LOCALVIEWPLAIN_H

#include <assert.h>

//!
//! \brief A plain struct to hold the content of the `PlainOffset` type defined in `LocalViewPlain.hpp`.
//!
//! We use a flexible array member here, as the actual size of the `data` member is known only in the C++ world.
//!
//! How to use:
//! ```
//! auto cpp_offset = ::HPM::internal::LocalView::CreatePlain(data, mesh, entity);
//! PlainOffset* offset = (PlainOffset*)(malloc(cpp_offset.byte_size));
//! std::memcpy(offset, &cpp_offset, cpp_offset.byte_size);
//! // offset can be used now!
//! ```
//!
typedef struct
{
    const unsigned cell_dimension;
    const unsigned num_entries;
    const unsigned long long size;
    const unsigned long long byte_size;
    const unsigned long long data[];
} PlainOffset;

typedef enum
{
    NodeEntity = 0,
    EdgeEntity,
    FaceEntity,
    CellEntity,
    Global
} EntityName;

[[maybe_unused]] static unsigned GetCellDimension(const PlainOffset* const plain_offset)
{
    assert(plain_offset != NULL);

    return plain_offset->cell_dimension;
}

[[maybe_unused]] static unsigned GetNumEntries(const PlainOffset* const plain_offset)
{
    assert(plain_offset != NULL);

    return plain_offset->num_entries;
}

[[maybe_unused]] static unsigned long long GetNumEntities(const PlainOffset* const plain_offset, const unsigned entry_index, EntityName entity)
{
    assert(plain_offset != NULL);
    assert(entry_index < (plain_offset->size));

    const unsigned entry_offset = plain_offset->data[entry_index];

    assert((entry_offset + entity) < (plain_offset->size));

    const unsigned long long position = plain_offset->data[entry_offset + entity];

    assert(position < (plain_offset->size));

    return plain_offset->data[position];
}

[[maybe_unused]] static const unsigned long long* GetOffsets(const PlainOffset* const plain_offset, const unsigned entry_index, EntityName entity)
{
    assert(plain_offset != NULL);
    assert(entry_index < (plain_offset->size));

    const unsigned entry_offset = plain_offset->data[entry_index];

    assert((entry_offset + entity) < (plain_offset->size));

    const unsigned long long position = plain_offset->data[entry_offset + entity];

    assert((position + 1) < (plain_offset->size));

    return &(plain_offset->data[position + 1]);
}

#endif