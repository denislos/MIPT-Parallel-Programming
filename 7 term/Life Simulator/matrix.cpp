
#include "matrix.h"

#include <omp.h>

#include <iostream>
#include <ctime>

Matrix::Matrix(std::size_t num_rows, std::size_t num_cols)
    : num_rows(num_rows)
    , num_cols(num_cols)
{
    matrixes.fill(std::vector<std::vector<bool>>(num_rows, std::vector<bool>(num_cols, false)));
    shape_matrix = std::vector<std::vector<sf::RectangleShape>>(num_rows, std::vector<sf::RectangleShape>(num_cols, sf::RectangleShape(sf::Vector2f(RECTANGLE_SIZE, RECTANGLE_SIZE))));

    for (auto i = 0u; i < num_rows; i++)
        for (auto j = 0u; j < num_cols; j++)
        {
            shape_matrix[i][j].setOutlineColor(sf::Color::Black);
            shape_matrix[i][j].setOutlineThickness(2);
            shape_matrix[i][j].setPosition(sf::Vector2f(SHAPE_MATRIX_HORIZONTAL_SHIFT + RECTANGLE_SIZE * j,
                                                        SHAPE_MATRIX_VERTICAL_SHIFT + RECTANGLE_SIZE * i));
        }
    /*
    matrixes[counter][27][14] = true;
    shape_matrix[27][14].setFillColor(sf::Color::Yellow);

    matrixes[counter][28][15] = true;
    shape_matrix[27][15].setFillColor(sf::Color::Yellow);

    matrixes[counter][27][16] = true;
    shape_matrix[27][16].setFillColor(sf::Color::Yellow);

    matrixes[counter][26][15] = true;
    shape_matrix[27][17].setFillColor(sf::Color::Yellow);
    */


    std::srand(std::time(nullptr));

    for (std::size_t j = 0; j < num_rows; j++)
        for (auto i = 0u; i < 50; i++)
        {
            const auto rand_idx = std::rand() % num_cols;

            matrixes[0][j][rand_idx] = true;
            shape_matrix[j][rand_idx].setFillColor(sf::Color::Yellow);
        }
}


void Matrix::draw(sf::RenderWindow& window) const
{
    for (const auto& row: shape_matrix)
        for (const auto& elem: row)
            window.draw(elem);
}


void Matrix::clock()
{
    # pragma omp parallel for schedule(dynamic)
    for (std::size_t i = 0; i < num_rows; i++)
        for (std::size_t j = 0; j < num_cols; j++)
        {
            unsigned int count = 0;

            if (i != 0)
            {
                if (j != 0)
                    count += matrixes[counter][i - 1][j - 1];
                
                if (j != num_cols - 1)
                    count += matrixes[counter][i - 1][j + 1];

                count += matrixes[counter][i - 1][j];
            }

            if (j != 0)
                count += matrixes[counter][i][j - 1];
            
            if (j != num_cols - 1)
                count += matrixes[counter][i][j + 1];
            
            if (i != num_rows - 1)
            {
                if (j != 0)
                    count += matrixes[counter][i + 1][j - 1];
                
                if (j != num_cols - 1)
                    count += matrixes[counter][i + 1][j + 1];
                
                count += matrixes[counter][i + 1][j];
            }


            if (!matrixes[counter][i][j] && count == 3)
            {
                matrixes[!counter][i][j] = true;
                shape_matrix[i][j].setFillColor(sf::Color::Yellow);
            }
            else if (matrixes[counter][i][j] && (count < 2 || count > 3))
            {
                matrixes[!counter][i][j] = false;
                shape_matrix[i][j].setFillColor(sf::Color::White);
            }
            else if (matrixes[counter][i][j])
            {
                matrixes[!counter][i][j] = true;
                shape_matrix[i][j].setFillColor(sf::Color::Yellow);
            }
            else
            {
                matrixes[!counter][i][j] = false;
                shape_matrix[i][j].setFillColor(sf::Color::White);
            }
            
        }

    counter = !counter;
}