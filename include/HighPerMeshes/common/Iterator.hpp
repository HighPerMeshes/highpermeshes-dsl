// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_ITERATOR_HPP
#define COMMON_ITERATOR_HPP

#include <cassert>
#include <iterator>
#include <set>
#include <type_traits>
#include <vector>

namespace HPM::iterator
{
    //!
    //! \brief A range that allows random access to a collection given a set of indices.
    //!
    //! \tparam Collection The underlying collection must define the function `at()`
    //!
    template <typename Collection>
    class RandomAccessRange
    {
        //!
        //! \brief Internal data structure to define an iterator over a RandomAccessRange.
        //!
        class RandomAccessIterator : std::iterator<std::input_iterator_tag, typename Collection::value_type>
        {
            public:
            RandomAccessIterator(Collection& collection, std::set<std::size_t>::const_iterator iterator) : collection{collection}, iterator{iterator} {}

            inline auto operator++() -> RandomAccessIterator&
            {
                iterator++;
                return *this;
            }

            inline auto operator++(int) -> RandomAccessIterator
            {
                auto it{*this};
                iterator++;
                return it;
            }

            inline auto& operator*() { return collection.at(*iterator); }

            friend auto operator==(const RandomAccessIterator& rhs, const RandomAccessIterator& lhs) -> bool { return rhs.iterator == lhs.iterator; }

            friend auto operator!=(const RandomAccessIterator& rhs, const RandomAccessIterator& lhs) -> bool { return !(rhs == lhs); }

            private:
            Collection& collection;
            std::set<std::size_t>::const_iterator iterator;
        };

        public:
        using value_type = typename Collection::value_type;

        RandomAccessRange(Collection& collection, std::set<std::size_t>&& indices) : collection{collection}, indices{std::move(indices)} {}
        RandomAccessRange(Collection& collection, const std::set<std::size_t>& indices) : collection{collection}, indices{indices} {}

        inline auto begin() const { return RandomAccessIterator{collection, indices.cbegin()}; }

        inline auto end() const { return RandomAccessIterator{collection, indices.cend()}; }

        inline auto size() const { return indices.size(); }

        private:
        Collection& collection;
        const std::set<std::size_t> indices;
    };

    //!
    //!
    //! \brief An contiguous iterator type.
    //!
    //! \tparam IntegerT the integer type to be used for the internal counter variable
    //! \tparam StepSize the value of the counter increment
    //!
    template <typename IntegerT, IntegerT StepSize = 1>
    struct Iterator : std::iterator<std::input_iterator_tag, IntegerT>
    {
        static_assert(std::is_integral_v<IntegerT>, "error: this is not an integer");

        //!
        //! \brief Constructor.
        //!
        //! \param value the initial value of the internal counter
        //!
        Iterator(const IntegerT value) : value(value) {}

        //!
        //! \brief Dereference operator.
        //!
        //! \return the value of the internal counter
        //!
        inline auto operator*() const -> IntegerT { return value; }

        //!
        //! \brief (Pre-)Increment operator.
        //!
        //! Increment the internal counter first, and then return this iterator.
        //!
        //! \return a reference to this iterator
        //!
        inline auto operator++() -> Iterator&
        {
            value += StepSize;
            return *this;
        }

        //!
        //! \brief (Post-)Increment operator.
        //!
        //! Increment the internal counter after having created a copy of this iterator.
        //!
        //! \return a copy of this iterator before the increment
        //!
        inline auto operator++(int)
        {
            Iterator it{*this};
            value += StepSize;
            return it;
        }

        //!
        //! \brief Comparison operator (equality).
        //!
        //! \param it a reference to another iterator
        //! \return `true` if this iterator and the other one have the same internal state, otherwise `false`
        //!
        //!
        inline auto operator==(const Iterator& it) const
        {
            // return (value >= it.value ? (value - it.value) < StepSize : (it.value - value) < StepSize);
            return (StepSize > 0 ? (value >= it.value) : (value < it.value));
        }

        //!
        //! \brief Comparison operator (inequality).
        //!
        //! \param it a reference to another iterator
        //! \return `false` if this iterator and the other one have the same internal state, otherwise `true`
        //!
        inline auto operator!=(const Iterator& it) const { return !(*this == it); }

        protected:
        IntegerT value;
    };

    //!
    //! \brief A contiguous range type.
    //!
    //! This data type implements a range of integers that can be iterated over
    //!
    //! \tparam IntegerT the integer type used for the internal iterators
    //! \tparam StepSize the value of the iterator increment
    //!
    template <typename IntegerT, IntegerT StepSize = 1>
    struct Range
    {
        static_assert(std::is_integral_v<IntegerT>, "error: this is not an integer");

        //!
        //! \brief Constructor.
        //!
        //! \param begin the begin of the iterator range
        //! \param end the end of the iterator range
        //!
        Range(const IntegerT begin, const IntegerT end) : it_begin(begin), it_end(end) { assert((end - begin) % StepSize == 0); }

        //!
        //! \brief Constructor.
        //!
        //! \param end the end of the iterator range
        //!
        Range(const IntegerT end) : Range(0, end) {}

        //!
        //! \brief Get the begin of this range.
        //!
        //! \return the begin of this range
        //!
        inline auto begin() const { return it_begin; }

        //!
        //! \brief Get the end of this range.
        //!
        //! \return the end of this range
        //!
        inline auto end() const { return it_end; }

        private:
        Iterator<IntegerT, StepSize> it_begin;
        Iterator<IntegerT, StepSize> it_end;
    };

    namespace internal
    {
        //!
        //! \brief A forward iterator over mesh entities.
        //!
        //! \tparam EntityT the type of the entity
        //! \tparam MeshT the mesh type
        //!
        template <typename EntityT_, typename MeshT>
        class EntityIterator
        {
            public:
            // Template arguments.
            using EntityT = EntityT_;

            //!
            //! \brief Constructor.
            //!
            //! Creates a forward iterator with its initial state set to `value`.
            //!
            //! \param mesh the associated mesh
            //! \param value the initial state of the iterator: used for the increment operation
            //! \param containing_mesh_entity_index the index of the containing mesh entity
            //!
            EntityIterator(const MeshT& mesh, const std::size_t value, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
                : mesh(mesh), value(value), containing_mesh_entity_index(containing_mesh_entity_index)
            {
            }

            //!
            //! \brief Increment operator.
            //!
            //! This function moves the iterator one position forward.
            //!
            //! \return a reference to itself
            //!
            inline auto operator++() -> EntityIterator&
            {
                ++value;
                return *this;
            }

            //!
            //! \brief Increment operator.
            //!
            //! This function moves the iterator multiple positions forward.
            //!
            //! \param increment the number of positions this iterator is moved forward
            //! \return a reference to itself
            //!
            inline auto operator+=(const std::size_t increment) -> EntityIterator&
            {
                value += increment;
                return *this;
            }

            //!
            //! \brief Comparison operator.
            //!
            //! \param it another entity iterator
            //! \return `true` if this iterator and `it` have the same state, otherwise `false`
            //!
            inline auto operator==(const EntityIterator& it) const { return (&mesh == &it.mesh) && (value == it.value); }

            //!
            //! \brief Comparison operator.
            //!
            //! \param it another entity iterator
            //! \return `false` if this iterator and `it` have the same state, otherwise `true`
            //!
            inline auto operator!=(const EntityIterator& it) const { return !(*this == it); }

            //!
            //! \brief Comparison operator.
            //!
            //! \param it another entity iterator
            //! \return `false` if this iterator is larger than `it`, otherwise `true`
            //!
            inline auto operator<(const EntityIterator& it) const { return (&mesh == &it.mesh) && (value < it.value); }

            //!
            //! \brief Dereference operation.
            //!
            //! This function creates an entity with the iterator state as local index and (global) index.
            //! Further, the containing mesh-entity index is forwarded to the entity.
            //!
            //! \return an entity according to the iterator state
            //!
            inline auto operator*() const -> EntityT { return {mesh, value, value, containing_mesh_entity_index}; }

            //!
            //! \brief Array subscript operator.
            //!
            //! This function creates an entity with the iterator state shifted by `index` as local index and (global) index.
            //! Further, the containing mesh-entity index is forwarded to the entity.
            //!
            //! \param index shift to be applied to the internal state
            //! \return an entity according to the iterator state shifted by `index`
            //!
            inline auto operator[](int index) const -> EntityT { return {mesh, value + index, value + index, containing_mesh_entity_index}; }

            protected:
            const MeshT& mesh;
            std::size_t value;
            const std::size_t containing_mesh_entity_index;
        };

        //!
        //! \brief A forward iterator over mesh entities.
        //!
        //! This iterator uses an index field for the entity creation.
        //! The iterator state specifies the local entity-index, whereas the corresponding entry in the index field specifies its index.
        //!
        //! Execptional case: the local entity-index equals its index iff
        //!     - the entity type is a cell (there is no embedding super structure then, and local index and (global) index must be equal)
        //!     - `AssignIndex` is set to `true`
        //!
        //! \tparam EntityT the type of the entity
        //! \tparam MeshT the mesh type
        //! \tparam AssignIndex a parameter to control the local entity-index assignment
        //!
        template <typename EntityT_, typename MeshT, bool AssignIndex = false>
        class IndexedEntityIterator : public EntityIterator<EntityT_, MeshT>
        {
            using Base = EntityIterator<EntityT_, MeshT>;

            public:
            // Template arguments.
            using EntityT = EntityT_;
            // Deduced types and constants.
            static constexpr bool IsCell = (EntityT::Dimension == MeshT::CellDimension);

            //!
            //! \brief Constructor.
            //!
            //! Creates a forward iterator with its initial state set to `value` and with an index field for the entity indexing.
            //! This constructor does not take ownership of the index set.
            //! It is used in all cases where a reference to a non-temporary field is provided.
            //!
            //! \param mesh the associated mesh
            //! \param value the initial state of the iterator: used for the increment operation
            //! \param index_set a container holding the indices of the entities
            //! \param containing_mesh_entity_index the index of the containing mesh entity
            //!
            IndexedEntityIterator(const MeshT& mesh, const std::size_t value, const std::vector<std::size_t>& index_set, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
                : Base(mesh, value, containing_mesh_entity_index), index_set(index_set)
            {
            }

            //!
            //! \brief Constructor.
            //!
            //! Creates a forward iterator with its initial state set to `value` and with an index field for the entity indexing.
            //! This constructor takes ownership of the index set.
            //! It is used in all cases where a temporary field is provided: a hard copy is created internally and referenced through `index_set` afterwards.
            //!
            //! \param mesh the associated mesh
            //! \param value the initial state of the iterator: used for the increment operation
            //! \param index_set a temporary container object holding the indices of the entities
            //! \param containing_mesh_entity_index the index of the containing mesh entity
            //!
            IndexedEntityIterator(const MeshT& mesh, const std::size_t value, std::vector<std::size_t>&& index_set, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
                : Base(mesh, value, containing_mesh_entity_index),
                    // Copy the index set.
                    index_set_internal(index_set),
                    // Set up a reference to the internal copy.
                    index_set(index_set_internal)
            {
            }

            //!
            //! \brief Increment operator.
            //!
            //! This function moves the iterator multiple positions forward.
            //!
            //! \param increment the number of positions this iterator is moved forward
            //! \return a reference to itself
            //!
            inline auto operator++() -> IndexedEntityIterator&
            {
                ++value;
                return *this;
            }

            //!
            //! \brief Increment operator.
            //!
            //! This function moves the iterator multiple positions forward.
            //!
            //! \param increment the number of positions this iterator is moved forward
            //! \return a reference to itself
            //!
            inline auto operator+=(const std::size_t increment) -> IndexedEntityIterator&
            {
                value += increment;
                return *this;
            }

            //!
            //! \brief Dereference operation.
            //!
            //! This function creates an entity with the iterator state as local index and the corresponding entry in the index field as (global) index.
            //! Further, the containing mesh-entity index is forwarded to the entity.
            //!
            //! \return an entity according to the iterator state and index field entry
            //!
            inline auto operator*() const -> EntityT
            {
                if constexpr (IsCell || AssignIndex)
                {
                    return {mesh, index_set[value], index_set[value], containing_mesh_entity_index};
                }
                else
                {
                    return {mesh, value, index_set[value], containing_mesh_entity_index};
                }
            }

            //!
            //! \brief Array subscript operator.
            //!
            //! This function creates an entity with the iterator state shifted by `index` as local index and the corresponding entry in the index field as (global) index.
            //! Further, the containing mesh-entity index is forwarded to the entity.
            //!
            //! \param index shift to be applied to the internal state
            //! \return an entity according to the iterator state shifted by `index`
            //!
            inline auto operator[](int index) const -> EntityT
            {
                if constexpr (IsCell || AssignIndex)
                {
                    return {mesh, index_set[value + index], index_set[value + index], containing_mesh_entity_index};
                }
                else
                {
                    return {mesh, value + index, index_set[value + index], containing_mesh_entity_index};
                }
            }

            protected:
            using Base::containing_mesh_entity_index;
            using Base::mesh;
            using Base::value;
            const std::vector<std::size_t> index_set_internal;
            const std::vector<std::size_t> index_set;
        };
    } // namespace internal

    //!
    //! \brief A forward iterator range over mesh entities.
    //!
    //! \tparam EntityT the type of the entity
    //! \tparam MeshT the mesh type
    //!
    template <typename EntityT_, typename MeshT>
    class EntityRange
    {
        public:
        // Template arguments.
        using EntityT = EntityT_;
        // Deduced types and constants.
        using IteratorT = internal::EntityIterator<EntityT, MeshT>;

        //!
        //! \brief Constructor.
        //!
        //! Creates a forward iterator range over entities with local index and (global) index between `begin` and `end`.
        //!
        //! \param mesh the associated mesh
        //! \param begin the begin of the range
        //! \param end the end of the range
        //! \param containing_mesh_entity_index the index of the containing mesh entity
        //!
        EntityRange(const MeshT& mesh, const std::size_t begin, const std::size_t end, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
            : it_begin(mesh, begin, containing_mesh_entity_index), it_end(mesh, end, containing_mesh_entity_index), range_size(end - begin)
        {
            assert(end >= begin);
        }

        //!
        //! \brief Get an iterator pointing to the begin of the range.
        //!
        //! \return an iterator pointing to the begin of the range
        //!
        inline auto begin() const { return it_begin; }

        //!
        //! \brief Get an iterator pointing to the end of the range.
        //!
        //! \return an iterator pointing to the end of the range
        //!
        inline auto end() const { return it_end; }

        //!
        //! \brief Get the extent of this range.
        //!
        //! \return the extent of this range
        //!
        inline auto GetRangeSize() const { return range_size; }

        private:
        const IteratorT it_begin;
        const IteratorT it_end;
        const std::size_t range_size;
    };

    //!
    //! \brief A forward iterator range over mesh entities.
    //!
    //! An index field for the entity creation is used.
    //!
    //! \tparam EntityT the type of the entity
    //! \tparam MeshT the mesh type
    //! \tparam AssignIndex a parameter to control the local entity-index assignment
    //!
    template <typename EntityT_, typename MeshT, bool AssignIndex = false>
    class IndexedEntityRange
    {
        public:
        // Template arguments.
        using EntityT = EntityT_;
        // Deduced types and constants.
        using IteratorT = internal::IndexedEntityIterator<EntityT, MeshT, AssignIndex>;

        //!
        //! \brief Constructor.
        //!
        //! Creates a forward iterator range over entities with local index between zero and the size of the index field, and (global) index according to the index field.
        //! The start index of the range is zero in all cases.
        //! The size of the index field specifies the end index of the iterator range.
        //!
        //! \param mesh the associated mesh
        //! \param index_set a container holding the indices of the entities
        //! \param containing_mesh_entity_index the index of the containing mesh entity
        //!
        IndexedEntityRange(const MeshT& mesh, const std::vector<std::size_t>& index_set, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
            : it_begin(mesh, 0, index_set, containing_mesh_entity_index), it_end(mesh, std::distance(index_set.begin(), index_set.end()), {}), range_size(index_set.size())
        {
        }

        //!
        //! \brief Constructor.
        //!
        //! Creates a forward iterator range over entities with local index between zero and the size of the index field, and (global) index according to the index field.
        //! The start index of the range is zero in all cases.
        //! The size of the index field specifies the end index of the iterator range.
        //!
        //! \param mesh the associated mesh
        //! \param index_set a temporary container holding the indices of the entities
        //! \param containing_mesh_entity_index the index of the containing mesh entity
        //!
        IndexedEntityRange(const MeshT& mesh, std::vector<std::size_t>&& index_set, const std::size_t containing_mesh_entity_index = MeshT::InvalidIndex)
            : it_begin(mesh, 0, std::move(index_set), containing_mesh_entity_index), it_end(mesh, std::distance(index_set.begin(), index_set.end()), {}), range_size(index_set.size())
        {
        }

        //!
        //! \brief Get an iterator pointing to the begin of the range.
        //!
        //! \return an iterator pointing to the begin of the range
        //!
        inline auto begin() const { return it_begin; }

        //!
        //! \brief Get an iterator pointing to the end of the range.
        //!
        //! \return an iterator pointing to the end of the range
        //!
        inline auto end() const { return it_end; }

        //!
        //! \brief Get the extent of this range.
        //!
        //! \return the extent of this range
        //!
        inline auto GetRangeSize() const { return range_size; }

        private:
        const IteratorT it_begin;
        const IteratorT it_end;
        const std::size_t range_size;
    };
} // namespace HPM::iterator

#endif