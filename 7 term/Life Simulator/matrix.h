

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <ostream>

#include <SFML/Graphics.hpp>


class Matrix
{
public:
    Matrix(std::size_t num_rows, std::size_t num_cols);
    
    void clock();

    void draw(sf::RenderWindow& window) const;

    friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
    {
        for (const auto& row : matrix.matrixes[matrix.counter])
        {
            for (const auto& elem: row)
                os << elem << " ";
            
            os << std::endl;
        }

        return os;
    }

private:
    static constexpr const std::size_t RECTANGLE_SIZE = 10;
    static constexpr const std::size_t SHAPE_MATRIX_HORIZONTAL_SHIFT = 8 * RECTANGLE_SIZE;
    static constexpr const std::size_t SHAPE_MATRIX_VERTICAL_SHIFT = 5 * RECTANGLE_SIZE;

    const std::size_t num_rows;
    const std::size_t num_cols;

    bool counter = false;

    std::array<std::vector<std::vector<bool>>, 2> matrixes;
    std::vector<std::vector<sf::RectangleShape>> shape_matrix;
};


#endif // MATRIX_H