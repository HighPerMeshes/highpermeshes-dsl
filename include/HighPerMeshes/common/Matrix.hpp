// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef COMMON_MATRIX_HPP
#define COMMON_MATRIX_HPP

#include <cstdint>
#include <iostream>

#include <HighPerMeshes/auxiliary/Math.hpp>
#include <HighPerMeshes/common/Vec.hpp>

namespace HPM::dataType
{
    namespace internal
    {
        //!
        //! \brief Base class implementation of an `M` by `N` matrix with elements of type `T`.
        //!
        //! This base class wraps a fixed size array of vectors, initializes the array of vectors and
        //! provides element access through the array subscript operator.
        //! The `data` member can also be directly accessed.
        //!
        //! \tparam T the type of matrix elements
        //! \tparam M the number of rows
        //! \tparam N the number of columns
        //!
        template <typename T, std::size_t M, std::size_t N>
        class MatrixBase
        {
            protected:
            MatrixBase() : data{} {}

            //!
            //! \brief Constructor.
            //!
            //! Vector elements are taken from the variadic argument list.
            //!
            //! \tparam Args variadic parameter pack
            //! \param args variadic argument list
            //!
            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == (M * N) && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            MatrixBase(Args&&... args) : data_plain{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            MatrixBase(const std::array<Vec<T, N>, M>& data) : data(data) {}

            public:
            //!
            //! \brief Array subscript operator.
            //!
            //! Returns the matrix row with the specified index.
            //!
            //! \param index row index
            //! \return a reference to the matrix row with the specified `index`
            //!
            inline auto operator[](const int index) -> Vec<T, N>& { return data[index]; }

            //!
            //! \brief Array subscript operator.
            //!
            //! Returns the matrix row with the specified index.
            //!
            //! \param index row index
            //! \return a constant reference to the matrix row with the specified `index`
            //!
            inline auto operator[](const int index) const -> const Vec<T, N>& { return data[index]; }

            //!
            //! \brief Get the number of rows.
            //!
            //! \return the number of rows
            //!
            inline constexpr auto NumRows() const { return M; }

            //!
            //! \brief Get the number of columns.
            //!
            //! \return the number of columns
            //!
            inline constexpr auto NumColumns() const { return N; }

            //!
            //! \brief Get the matrix size.
            //!
            //! The matrix size is returned as a pair: (rows, columns).
            //!
            //! \return a pair containing the number of rows and columns
            //!
            inline constexpr auto size() const -> std::pair<std::size_t, std::size_t> { return {M, N}; }

            union {
                std::array<Vec<T, N>, M> data;
                std::array<T, N * M> data_plain;
            };
        };

        //!
        //! \brief Specialization of the base class for `N=3, M=3`.
        //!
        //! In addition to the `data` member, this specialization provides accees to the
        //! data through member variables `xx,xy,xz,yx,yy,yz,zx,zy,zz`.
        //!
        //! \tparam T the type of matrix elements
        //!
        template <typename T>
        class MatrixBase<T, 3, 3>
        {
            protected:
            MatrixBase() : data{} {}

            template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == 9 && std::conjunction_v<std::is_convertible<Args, T>...>), int> = 0>
            MatrixBase(Args&&... args) : data_plain{static_cast<T>(std::forward<Args>(args))...}
            {
            }

            MatrixBase(const std::array<Vec<T, 3>, 3>& data) : data(data) {}

            public:
            inline auto operator[](const int index) -> Vec<T, 3>& { return data[index]; }

            inline auto operator[](const int index) const -> const Vec<T, 3>& { return data[index]; }

            inline constexpr auto NumRows() const { return 3; }

            inline constexpr auto NumColumns() const { return 3; }

            inline constexpr auto size() const -> std::pair<std::size_t, std::size_t> { return {3, 3}; }

            // Access to the data either through 'data[][]' or 'xx,xy,..,zz'
            union {
                struct
                {
                    T xx, xy, xz, yx, yy, yz, zx, zy, zz;
                };
                std::array<Vec<T, 3>, 3> data;
                std::array<T, 3 * 3> data_plain;
            };
        };
    } // namespace internal

    //!
    //! \brief Matrix inversion schemes.
    //!
    enum class MatrixInversionScheme
    {
        Adjoint,
        Gauss
    };

    //!
    //! \brief Matrix implementation.
    //!
    //! Inherits from 'MatrixBase' -> provide 'xx|xy|..|zz' data access for N=3
    //! The implementation uses the 'data' member for data access.
    //!
    //! \tparam T the type of matrix elements
    //! \tparam M_ the number of rows
    //! \tparam N_ the number of columns
    //!
    template <typename T, std::size_t M_, std::size_t N_>
    class Matrix : public internal::MatrixBase<T, M_, N_>
    {
        using Base = internal::MatrixBase<T, M_, N_>;

        public:
        // Template arguments
        using ValueT = T;
        static constexpr std::size_t M = M_;
        static constexpr std::size_t N = N_;

        Matrix() : Base() {}

        Matrix(const Matrix& other) : Base(other.data) {}

        template <typename... Args, typename std::enable_if_t<(sizeof...(Args) == (M * N)), int> = 0>
        Matrix(Args&&... args) : Base(std::forward<Args>(args)...)
        {
        }

        // Operator definitions.
#define MACRO(OP)                                                                                                                                                                                                          \
inline auto operator OP(const Matrix& m)->Matrix&                                                                                                                                                                      \
{                                                                                                                                                                                                                      \
    for (std::size_t i = 0; i < M; ++i)                                                                                                                                                                                \
    {                                                                                                                                                                                                                  \
        for (std::size_t j = 0; j < N; ++j)                                                                                                                                                                            \
        {                                                                                                                                                                                                              \
            data[i][j] OP m[i][j];                                                                                                                                                                                     \
        }                                                                                                                                                                                                              \
    }                                                                                                                                                                                                                  \
                                                                                                                                                                                                                        \
    return *this;                                                                                                                                                                                                      \
}

        MACRO(=)
        MACRO(+=)
        MACRO(-=)

#undef MACRO

#define MACRO(OP)                                                                                                                                                                                                          \
inline auto operator OP(const T& x)->Matrix&                                                                                                                                                                           \
{                                                                                                                                                                                                                      \
    for (std::size_t i = 0; i < M; ++i)                                                                                                                                                                                \
    {                                                                                                                                                                                                                  \
        for (std::size_t j = 0; j < N; ++j)                                                                                                                                                                            \
        {                                                                                                                                                                                                              \
            data[i][j] OP x;                                                                                                                                                                                           \
        }                                                                                                                                                                                                              \
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

        //!
        //! \brief Negation operator.
        //!
        //! \return negative of this matrix
        //!
        inline auto operator-() const -> Matrix
        {
            Matrix matrix{*this};

            return (matrix * static_cast<T>(-1));
        }

        //!
        //! \brief Comparison operator.
        //!
        //! \param m another matrix object
        //! \return `true` if `m` equals this matrix element-wise, otherwise `false`
        //!
        inline auto operator==(const Matrix& m) const
        {
            T sum{};

            for (std::size_t i = 0; i < M; ++i)
            {
                for (std::size_t j = 0; j < N; ++j)
                {
                    sum += (data[i][j] == m[i][j] ? 0 : 1);
                }
            }

            return (sum == 0);
        }

        //!
        //! \brief Matrix transpose operation.
        //!
        //! \return the transposed matrix
        //!
        inline auto Transpose() const
        {
            Matrix<T, N, M> matrix{};

            for (std::size_t i = 0; i < M; ++i)
            {
                for (std::size_t j = 0; j < N; ++j)
                {
                    matrix[j][i] = data[i][j];
                }
            }

            return matrix;
        }

        //!
        //! \brief Matrix determinant calculation.
        //!
        //! The determinant of a square matrix is calculated according to the Leibniz formula.
        //!
        //! \return the matrix determinant
        //!
        inline auto Determinant() const
        {
            static_assert(M == N, "error: determinant cannot be calculated for M != N");

            T determinant = 0;

            // Leibniz' rule
            for (std::size_t s = 0; s < ::HPM::math::Factorial(N); ++s)
            {
                const std::array<std::size_t, N>& permutation = ::HPM::math::GetPermutation<N>(s);
                T product = ::HPM::math::GetSignOfPermutation<N>(s);

                for (std::size_t i = 0; i < N; ++i)
                {
                    product *= data[i][permutation[i]];
                }

                determinant += product;
            }

            return determinant;
        }

        //!
        //! \brief Matrix determinant calculation.
        //!
        //! The determinant of a square matrix is calculated according to the Leibniz formula.
        //!
        //! \param m a matrix representation
        //! \return the matrix determinant
        //!
        static inline auto Determinant(const std::array<std::array<T, N>, M>& m)
        {
            Matrix matrix{m};

            return matrix.Determinant();
        }

        //!
        //! \brief Inverse matrix calculation.
        //!
        //! The inversion of the matrix \f$A\f$ is calculated either by the adjoint matrix
        //! (\f$A^{-1} = adj(A)/det(A)\f$) or by the Gauss-Jordan Elimination.
        //!
        //! \return the inverse of the Matrix
        //!
        template <MatrixInversionScheme Scheme = MatrixInversionScheme::Adjoint>
        inline auto Invert() const -> Matrix
        {
            static_assert(M == N, "error: M and N differ");
            static_assert(M > 0 && M < 4, "error: M (or N) must be any of 1..3.");

            if constexpr (Scheme == MatrixInversionScheme::Adjoint)
            {
                const T det = Determinant();

                // \todo { cout should be some kind of error - Stefan G. 6.11.2019 }
                if (det == 0.0)
                {
                    std::cout << "error in Matrix::Invert() : Determinant is zero -> inverse does not exist" << std::endl << std::flush;
                }

                const T inv_det = 1 / det;

                if constexpr (M == 2)
                {
                    return {data[1][1] * inv_det, -data[0][1] * inv_det, -data[1][0] * inv_det, data[0][0] * inv_det};
                }
                else if constexpr (M == 3)
                {
                    // Sarrus' rule
                    const T a = data[0][0];
                    const T b = data[0][1];
                    const T c = data[0][2];
                    const T d = data[1][0];
                    const T e = data[1][1];
                    const T f = data[1][2];
                    const T g = data[2][0];
                    const T h = data[2][1];
                    const T i = data[2][2];

                    return {(e * i - f * h) * inv_det, (c * h - b * i) * inv_det, (b * f - c * e) * inv_det, (f * g - d * i) * inv_det, (a * i - c * g) * inv_det,
                            (c * d - a * f) * inv_det, (d * h - e * g) * inv_det, (b * g - a * h) * inv_det, (a * e - b * d) * inv_det};
                }
            }
            else
            {
                std::cerr << "error in Matrix::Invert() : use Adjoint" << std::endl << std::flush;
            }
        }

        using Base::data;
    };

    // Operator definitions.
#define MACRO(OP)                                                                                                                                                                                                          \
template <typename T, std::size_t M, std::size_t N>                                                                                                                                                                    \
inline auto operator OP(const Matrix<T, M, N>& m_1, const Matrix<T, M, N>& m_2)                                                                                                                                        \
{                                                                                                                                                                                                                      \
    Matrix<T, M, N> matrix{m_1};                                                                                                                                                                                       \
                                                                                                                                                                                                                        \
    matrix OP## = m_2;                                                                                                                                                                                                 \
                                                                                                                                                                                                                        \
    return matrix;                                                                                                                                                                                                     \
}

    MACRO(+)
    MACRO(-)

#undef MACRO

#define MACRO(OP)                                                                                                                                                                                                          \
template <typename T, std::size_t M, std::size_t N>                                                                                                                                                                    \
inline auto operator OP(const Matrix<T, M, N>& m, const T x)                                                                                                                                                           \
{                                                                                                                                                                                                                      \
    Matrix<T, M, N> matrix{m};                                                                                                                                                                                         \
                                                                                                                                                                                                                        \
    matrix OP## = x;                                                                                                                                                                                                   \
                                                                                                                                                                                                                        \
    return matrix;                                                                                                                                                                                                     \
}                                                                                                                                                                                                                      \
                                                                                                                                                                                                                        \
template <typename T, std::size_t M, std::size_t N>                                                                                                                                                                    \
inline auto operator OP(const T x, const Matrix<T, M, N>& m)                                                                                                                                                           \
{                                                                                                                                                                                                                      \
    Matrix<T, M, N> matrix{m};                                                                                                                                                                                         \
                                                                                                                                                                                                                        \
    matrix OP## = x;                                                                                                                                                                                                   \
                                                                                                                                                                                                                        \
    return matrix;                                                                                                                                                                                                     \
}

    MACRO(+)
    MACRO(-)
    MACRO(*)
    MACRO(/)

#undef MACRO

    //!
    //! \brief Matrix-Matrix multiplication.
    //!
    //! Multiply two matrices `LxM`*`MxN` to obtain a `LxN` matrix.
    //!
    //! \tparam T the type of matrix elements
    //! \tparam M the number of rows of mat_1
    //! \tparam K the number of columns of mat_1 and the number of rows of mat_2
    //! \tparam N the number of columns of mat_2
    //! \param mat_1 a `MxK` matrix
    //! \param mat_1 a `KxN` matrix
    //! \return a `MxN` matrix
    //!
    template <typename T, std::size_t M, std::size_t K, std::size_t N>
    inline auto operator*(const Matrix<T, M, K>& mat_1, const Matrix<T, K, N>& mat_2)
    {
        Matrix<T, M, N> matrix{};

        for (std::size_t i = 0; i < M; ++i)
        {
            for (std::size_t k = 0; k < K; ++k)
            {
                for (std::size_t j = 0; j < N; ++j)
                {
                    matrix[i][j] += mat_1[i][k] * mat_2[k][j];
                }
            }
        }

        return matrix;
    }

    //!
    //! \brief Matrix-Vector multiplication.
    //!
    //! \tparam T the type of vector and matrix elements
    //! \tparam M the number of matrix rows
    //! \tparam N the number of matrix columns and vector elements
    //! \param m a `MxN` matrix
    //! \param v a vector with `N` elements
    //! \return a vector with `M` elements
    //!
    template <typename T, std::size_t M, std::size_t N>
    inline auto operator*(const Matrix<T, M, N>& m, const Vec<T, N>& v)
    {
        Vec<T, M> vector{};

        for (std::size_t i = 0; i < M; ++i)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                vector[i] += m[i][j] * v[j];
            }
        }

        return vector;
    }

    //!
    //! \brief Vector-Matrix multiplication.
    //!
    //! \tparam T the type of vector and matrix elements
    //! \tparam M the number of matrix rows and vector elements
    //! \tparam N the number of matrix columns
    //! \param v a vector with `M` elements
    //! \param m a `MxN` matrix
    //! \return a vector with `N` elements
    //!
    template <typename T, std::size_t M, std::size_t N>
    inline auto operator*(const Vec<T, M>& v, const Matrix<T, M, N>& m)
    {
        Vec<T, N> vector{};

        for (std::size_t i = 0; i < M; ++i)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                vector[j] += v[i] * m[i][j];
            }
        }

        return vector;
    }

    //!
    //! \brief Matrix transpose operation.
    //!
    //! \tparam T the type of the matrix elements
    //! \tparam M the number of matrix rows
    //! \tparam N the number of matrix columns
    //! \param m a `MxN` matrix
    //! \return the transposed matrix
    //!
    template <typename T, std::size_t M, std::size_t N>
    inline auto Transpose(const Matrix<T, M, N>& m)
    {
        return m.Transpose();
    }

    //!
    //! \brief Curl operation for `M=N=3`.
    //!
    //! \tparam T the type of matrix elements
    //! \param m_1 a `3x3` matrix
    //! \param m_2 a `3x3` matrix
    //! \return the result of curl operation
    //!
    template <typename T>
    inline auto Curl(const Matrix<T, 3, 3>& m_1, const Matrix<T, 3, 3>& m_2) -> Vec<T, 3>
    {
        const Matrix<T, 3, 3>& m_1t{m_1.Transpose()};

        return {(m_1t[1] * m_2[2]) - (m_1t[2] * m_2[1]), (m_1t[2] * m_2[0]) - (m_1t[0] * m_2[2]), (m_1t[0] * m_2[1]) - (m_1t[1] * m_2[0])};
    }

    //!
    //! \brief Dyadic product (tensor product, outer product).
    //!
    //! Multiply two vectors `Mx1`*`1xN` to obtain a `NxM` matrix.
    //!
    //! \tparam T the type of vector and matrix elements
    //! \tparam M, N the number of elements in the vectors
    //! \param v_1 a vector with `M` elements
    //! \param v_2 a vector with `N` elements
    //! \return a `NxM` matrix
    //!
    template <typename T, std::size_t M, std::size_t N>
    inline auto DyadicProduct(const Vec<T, M>& v_1, const Vec<T, N>& v_2)
    {
        Matrix<T, N, M> matrix{};

        for (std::size_t j = 0; j < N; ++j)
        {
            for (std::size_t i = 0; i < M; ++i)
            {
                matrix[j][i] = v_2[j] * v_1[i];
            }
        }

        return matrix;
    }

    template <typename T, std::size_t N, size_t M>
    auto operator<<(std::ostream& output_stream, const Matrix<T, N, M>& mat) -> std::ostream&
    {
        output_stream << "{ ";
        for (const auto& vec : mat.data)
        {
            output_stream << vec << " ";
        }
        output_stream << "}";

        return output_stream;
    }
} // namespace HPM::dataType

#endif