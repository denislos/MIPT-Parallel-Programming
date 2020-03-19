

#ifndef MATRIX_H
#define MATRIX_H

#include <array>
#include <iostream>
#include <cstring>

class Calculator;

static constexpr const std::size_t NUM_COLUMNS = 10;
static constexpr const std::size_t NUM_ROWS = 10;


template <std::size_t NROWS, std::size_t NCOLS>
class Matrix
{
public:
    Matrix(double lt, double ut, double rt, double dt);

    friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
    {
        for (auto i = 0u; i < NROWS; i++)
        {
            for (auto j = 0u; j < NCOLS - 1; j++)
                os << matrix.data[i][j] << " ";

            os << matrix.data[i][NCOLS - 1] << std::endl;
        }

        return os;
    }

    Matrix() = delete;
    Matrix(const Matrix& rhs) = delete;
    Matrix(Matrix&& rhs) = delete;
    Matrix& operator=(const Matrix& rhs)
    {
        memcpy(&data[0][0], &rhs.data[0][0], NROWS * NCOLS * sizeof(double));
        return *this;
    }
    Matrix& operator=(Matrix&& rhs) = delete;

private:
    double data[NROWS][NCOLS];
    
    friend class Calculator;
};


template <std::size_t NROWS, std::size_t NCOLS>
Matrix<NROWS, NCOLS>::Matrix(double lt, double ut, double rt, double dt)
{

    for (auto i = 1u; i < NROWS - 1; i++)
        for (auto j = 1u; j < NCOLS - 1; j++)
            data[i][j] = 0.25 * (lt + ut + rt + dt);

    for (auto j = 1u; j < NCOLS - 1; j++)
    {
        data[0][j] = ut;
        data[NROWS - 1][j] = dt;
    }

    for (auto i = 1u; i < NROWS - 1; i++)
    {
        data[i][0] = lt;
        data[i][NCOLS - 1] = rt;
    }
    
    data[0][0] = 0.5 * (lt + ut);
    data[0][NCOLS - 1] = 0.5 * (ut + rt);
    data[NROWS - 1][0] = 0.5 * (lt + dt);
    data[NROWS - 1][NCOLS - 1] = 0.5 * (dt + rt);
 }



#endif // MATRIX_H