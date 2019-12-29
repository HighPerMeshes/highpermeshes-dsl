// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_VEC_HPP
#define COMMON_VEC_HPP

#include <cstdint>
#include <ostream>
#include <utility>

#include <HighPerMeshes/auxiliary/Math.hpp>
#include <HighPerMeshes/common/TypeTraits.hpp>

namespace HPM::dataType
{
    namespace internal
    {
        //!
        //! \brief Base class implementation of a vector with `N` elements of type `T`.
        //!
        //! This base class wraps a fixed size array, initializes the array and
        //! provides element access through the array subscript operator.
        //! The `data` member can also be directly accessed.
        //!
        //! \tparam T the type of the vector elements
        //! \tparam N the number of vector elements
        //!
        template <typename T, std::size_t N>
        class VecBase
        {
            public:
            VecBase() : data{} {}

            VecBase(const std::array<T, N>& data) : data(data) {}

            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == N && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            VecBase(Args&&... args) : data{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            //!
            //! \brief Array subscript operator.
            //!
            //! Returns the array element at the specified position.
            //!
            //! \param index array position
            //! \return a reference to the array element at position `index`
            //!
            inline auto operator[](const int index) -> T& { return data[index]; }

            //!
            //! \brief Array subscript operator.
            //!
            //! Returns the array element at the specified position.
            //!
            //! \param index array position
            //! \return a constant reference to the array element at position `index`
            //!
            inline auto operator[](const int index) const -> const T& { return data[index]; }

            std::array<T, N> data;
        };

        //!
        //! \brief Specialization of the base class with 1 element.
        //!
        //! In addition to the `data` member, this specialization provides accees to the
        //! data through member variable `x`.
        //!
        //! \tparam T the type of the vector elements
        //!
        template <typename T>
        class VecBase<T, 1>
        {
            public:
            VecBase() : data{} {}

            VecBase(const std::array<T, 1>& data) : data(data) {}

            template <typename X>
            VecBase(X&& data) : data{data}
            {
            }

            inline auto operator[](const int index) -> T& { return data[index]; }

            inline auto operator[](const int index) const -> const T& { return data[index]; }

            // Access to the data either through 'data[0]' or 'x'
            union {
                std::array<T, 1> data;
                struct
                {
                    T x;
                };
            };
        };

        //!
        //! \brief Specialization of the base class with 2 elements.
        //!
        //! In addition to the `data` member, this specialization provides accees to the
        //! data through member variables `x` and `y`.
        //!
        //! \tparam T the type of the vector elements
        //!
        template <typename T>
        class VecBase<T, 2>
        {
            public:
            VecBase() : data{} {}

            VecBase(const std::array<T, 2>& data) : data(data) {}

            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == 2 && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            VecBase(Args&&... args) : data{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            inline auto operator[](const int index) -> T& { return data[index]; }

            inline auto operator[](const int index) const -> const T& { return data[index]; }

            // Access to the data either through 'data[0|1]' or 'x|y'
            union {
                std::array<T, 2> data;
                struct
                {
                    T x, y;
                };
            };
        };

        //!
        //! \brief Specialization of the base class with 3 elements.
        //!
        //! In addition to the `data` member, this specialization provides accees to the
        //! data through member variables `x`, `y` and `z`.
        //!
        //! \tparam T the type of the vector elements
        //!
        template <typename T>
        class VecBase<T, 3>
        {
            public:
            VecBase() : data{} {}

            VecBase(const std::array<T, 3>& data) : data(data) {}

            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == 3 && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            VecBase(Args&&... args) : data{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            inline auto operator[](const int index) -> T& { return data[index]; }

            inline auto operator[](const int index) const -> const T& { return data[index]; }

            // Access to the data either through 'data[0|1|2]' or 'x|y|z'
            union {
                std::array<T, 3> data;
                struct
                {
                    T x, y, z;
                };
            };
        };

        //!
        //! \brief Specialization of the base class with 4 elements.
        //!
        //! In addition to the `data` member, this specialization provides accees to the
        //! data through member variables `x``, `y`, `z` and `w`.
        //!
        //! \tparam T the type of the vector elements
        //!
        template <typename T>
        class VecBase<T, 4>
        {
            public:
            VecBase() : data{} {}

            VecBase(const std::array<T, 4>& data) : data(data) {}

            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == 4 && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            VecBase(Args&&... args) : data{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            inline auto operator[](const int index) -> T& { return data[index]; }

            inline auto operator[](const int index) const -> const T& { return data[index]; }

            // Access to the data either through 'data[0|1|2|3]' or 'x|y|z|w'
            union {
                std::array<T, 4> data;
                struct
                {
                    T x, y, z, w;
                };
            };
        };
    } // namespace internal

    //!
    //! \brief Vector implementation.
    //!
    //! The implementation inherits from `VecBase` and provides data access through `x,y,z,w` for `N=1,2,3,4`.
    //! The implementation uses the `data` member for data access.
    //!
    //! \tparam T type of the vector elements
    //! \tparam N_ number of vector elements
    //!
    template <typename T, std::size_t N_>
    class Vec : public internal::VecBase<T, N_>
    {
        static_assert(N_ > 0, "error: N must be at least 1");

        using Base = internal::VecBase<T, N_>;

        public:
        // Template arguments.
        using ValueT = T;
        static constexpr std::size_t N = N_;

        // Deduced types and constants.
        static constexpr std::size_t Dimension = N;

        Vec() : Base() {}

        Vec(const Vec& other) : Base(other.data) {}

        template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == N), int> = 0>
        Vec(Args&&... args) : Base(std::forward<Args>(args)...)
        {
        }

        // Operator definitions.
#define MACRO(OP)                                                                                                                                                                                                          \
inline auto operator OP(const Vec& v)->Vec&                                                                                                                                                                            \
{                                                                                                                                                                                                                      \
    for (std::size_t i = 0; i < N; ++i)                                                                                                                                                                                \
    {                                                                                                                                                                                                                  \
        data[i] OP v.data[i];                                                                                                                                                                                          \
    }                                                                                                                                                                                                                  \
                                                                                                                                                                                                                        \
    return *this;                                                                                                                                                                                                      \
}                                                                                                                                                                                                                      \
                                                                                                                                                                                                                        \
inline auto operator OP(const T s)->Vec&                                                                                                                                                                               \
{                                                                                                                                                                                                                      \
    for (std::size_t i = 0; i < N; ++i)                                                                                                                                                                                \
    {                                                                                                                                                                                                                  \
        data[i] OP s;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                                  \
                                                                                                                                                                                                                        \
    return *this;                                                                                                                                                                                                      \
}

        MACRO(=)
        MACRO(+=)
        MACRO(-=)
        MACRO(*=)
        MACRO(/=)

#undef MACRO

        inline auto operator-() const
        {
            Vec vector{*this};

            return (vector * static_cast<T>(-1));
        }

        inline auto operator==(const Vec& v) const
        {
            T sum = 0;

            for (std::size_t i = 0; i < N; ++i)
            {
                sum += (data[i] == v.data[i] ? 0 : 1);
            }

            return (sum == 0);
        }

        inline auto Norm() const
        {
            T norm = 0;

            for (std::size_t i = 0; i < N; ++i)
            {
                norm += (data[i] * data[i]);
            }

            return std::sqrt(norm);
        }

        using Base::data;
    };

    //!
    //! \brief Operator definitions.
    //!
#define MACRO(OP)                                                                                                                                                                                                          \
template <typename T, std::size_t N>                                                                                                                                                                                   \
inline auto operator OP(const Vec<T, N>& v_1, const Vec<T, N>& v_2)                                                                                                                                                    \
{                                                                                                                                                                                                                      \
    Vec<T, N> vector{v_1};                                                                                                                                                                                             \
    vector OP## = v_2;                                                                                                                                                                                                 \
    return vector;                                                                                                                                                                                                     \
}

    MACRO(+)
    MACRO(-)

#undef MACRO

#define MACRO(OP)                                                                                                                                                                                                          \
template <typename T, std::size_t N>                                                                                                                                                                                   \
inline auto operator OP(const Vec<T, N>& v_1, const T s)                                                                                                                                                               \
{                                                                                                                                                                                                                      \
    Vec<T, N> vector{v_1};                                                                                                                                                                                             \
    vector OP## = s;                                                                                                                                                                                                   \
    return vector;                                                                                                                                                                                                     \
}                                                                                                                                                                                                                      \
                                                                                                                                                                                                                        \
template <typename T, std::size_t N>                                                                                                                                                                                   \
inline auto operator OP(const T s, const Vec<T, N>& v_1)                                                                                                                                                               \
{                                                                                                                                                                                                                      \
    Vec<T, N> vector{v_1};                                                                                                                                                                                             \
    vector OP## = s;                                                                                                                                                                                                   \
    return vector;                                                                                                                                                                                                     \
}

    MACRO(+)
    MACRO(-)
    MACRO(*)
    MACRO(/)

#undef MACRO

    //!
    //! \brief Cross product for `N=3`.
    //!
    //! This implementation is for three-dimensional vectors only.
    //!
    //! \tparam T the type of vector elements
    //! \param v_1 a '3x1' vector
    //! \param v_2 a '3x1' vector
    //! \return the cross product of `v_1` and `v_2`
    //!
    template <typename T>
    inline auto CrossProduct(const Vec<T, 3>& v_1, const Vec<T, 3>& v_2) -> Vec<T, 3>
    {
        return {v_1.data[1] * v_2.data[2] - v_1.data[2] * v_2.data[1], v_1.data[2] * v_2.data[0] - v_1.data[0] * v_2.data[2], v_1.data[0] * v_2.data[1] - v_1.data[1] * v_2.data[0]};
    }

    //!
    //! \brief Scalar product.
    //!
    //! Multiply two vectors `1xN`*`Nx1` to obtain a scalar.
    //!
    //! \tparam T the type of vector and matrix elements
    //! \tparam N the number of elements in the vectors
    //! \param v_1 a vector with `N` elements
    //! \param v_2 a vector with `N` elements
    //! \return a scalar
    //!
    template <typename T, std::size_t N>
    inline auto operator*(const Vec<T, N>& v_1, const Vec<T, N>& v_2)
    {
        T dot_product = 0;

        for (std::size_t i = 0; i < N; ++i)
        {
            dot_product += (v_1.data[i] * v_2.data[i]);
        }

        return dot_product;
    }

    //!
    //! \brief Vector normalization.
    //!
    //! \tparam T the type of vector elements
    //! \tparam N the number of vector elements
    //! \param v the vector to be normalized
    //! \return the normalized `v`
    //!
    template <typename T, std::size_t N>
    inline auto Normalize(const Vec<T, N>& v)
    {
        return (v / v.Norm());
    }

    //!
    //! \brief Fill vector from an input stream.
    //!
    //! \tparam T the type of vector elements
    //! \tparam N the number of vector elements
    //! \param input_stream a data stream
    //! \param v the vector to be filled
    //! \return the input stream
    //!
    template <typename T, std::size_t N>
    auto operator>>(std::istream& input_stream, Vec<T, N>& v) -> std::istream&
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            input_stream >> v.data[i];
        }

        return input_stream;
    }

    //!
    //! \brief Write alle elements of a vector into an output stream.
    //!
    //! \tparam T the type of vector elements
    //! \tparam N the number of vector elements
    //! \param output_stream a data stream
    //! \param v the vector to be written
    //! \return the output stream
    //!
    template <typename T, std::size_t N>
    auto operator<<(std::ostream& output_stream, const Vec<T, N>& v) -> std::ostream&
    {
        output_stream << "{ ";
        for (std::size_t i = 0; i < N; ++i)
        {
            output_stream << v.data[i] << " ";
        }
        output_stream << "}";

        return output_stream;
    }

    namespace internal
    {
        //!
        //! \brief The `Vec<T, N>` type provides meta data.
        //!
        //! Meta data needed by the Mesh implementation: `C_Dimension`.
        //!
        template <typename T, std::size_t N>
        struct ProvidesMetaData<::HPM::dataType::Vec<T, N>>
        {
            static constexpr bool value = true;
        };
    } // namespace internal
} // namespace HPM::dataType

#endif