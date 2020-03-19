
#include <cstdlib>
#include <iostream>
#include <cstring>

#include <mpi.h>

#include "matrix.h"

static constexpr const double LEFT_TEMPERATURE  = 100;
static constexpr const double UP_TEMPERATURE    = 200;
static constexpr const double RIGHT_TEMPERATURE = 300;
static constexpr const double DOWN_TEMPERATURE  = 400;

using MatrixType = Matrix<NUM_ROWS, NUM_COLUMNS>;


static constexpr const std::size_t NUM_ROWS_TO_START_CALCULATION = 2;


static constexpr const char* const SEIDEL_PAR_STRING = "--Seidel";
static constexpr const char* const JACOBI_PAR_STRING = "--Jacobi";

static constexpr const std::size_t PAR_STRING_SIZE = 8;

class Calculator
{
public:
    static inline double calculate_seidel(const MatrixType& matrix, const MatrixType& matrix_new, std::size_t i, std::size_t j)
    {
        return 0.25 * (matrix.data[i - 1][j] + matrix_new.data[i + 1][j] + matrix_new.data[i][j - 1] + matrix.data[i][j + 1]);
    }

    static inline double calculate_jacobi(const MatrixType& matrix, const MatrixType& matrix_new, std::size_t i, std::size_t j)
    {
        return 0.25 * (matrix.data[i - 1][j] + matrix.data[i + 1][j] + matrix.data[i][j - 1] + matrix.data[i][j + 1]);
    }

    static void calculate(MatrixType& matrix, MatrixType& matrix_new, 
                          double (*calc_routine)(const MatrixType& matrix, const MatrixType& matrix_new, std::size_t i, std::size_t j), 
                          int rank, int num_procs, std::size_t num_iterations);
};



void Calculator::calculate(MatrixType& matrix, 
                           MatrixType& matrix_new, 
                           double (*calc_routine)(const MatrixType& matrix, const MatrixType& matrix_new, std::size_t i, std::size_t j), 
                           int rank, int num_procs, std::size_t num_iterations)
{
    const int receive_rank = (rank == 0) ? (num_procs - 1) : (rank - 1);
    const int send_rank = (rank == num_procs - 1) ? 0 : (rank + 1);

    for (std::size_t current_iteration = rank; current_iteration < num_iterations; current_iteration += num_procs)
    {
        std::size_t left_calculation_row  = 1;
        std::size_t right_calculation_row = NUM_ROWS_TO_START_CALCULATION;

        std::size_t left_receive_row = 1;
        std::size_t right_receive_row = NUM_ROWS_TO_START_CALCULATION;

        bool has_sent_all = false;
        bool has_received = (current_iteration == 0 && rank == 0);

        bool should_receive = (num_procs != 1);
        bool should_send    = (num_procs != 1);
        
        while (!has_sent_all)
        {
            if (should_receive && !has_received)
            {
                MPI_Recv(&matrix.data[left_receive_row][0], NUM_ROWS_TO_START_CALCULATION * NUM_COLUMNS,
                         MPI_DOUBLE, receive_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            if (right_receive_row < NUM_ROWS - 2)
            {
                left_receive_row = right_receive_row + 1;
                right_receive_row = left_receive_row + NUM_ROWS_TO_START_CALCULATION - 1;
            }
            else
                should_receive = false;



            for (auto i = left_calculation_row; i <= right_calculation_row; i++)
                for (auto j = 1u; j < NUM_COLUMNS - 1; j++)
                    matrix_new.data[i][j] = calc_routine(matrix, matrix_new, i, j);

            
            if (should_send)
            {
                MPI_Send(&matrix_new.data[left_calculation_row][0], NUM_ROWS_TO_START_CALCULATION * NUM_COLUMNS,
                         MPI_DOUBLE, send_rank, 0, MPI_COMM_WORLD);
            }


            if (right_calculation_row < NUM_ROWS - 2)
            {
                left_calculation_row = right_calculation_row + 1;
                right_calculation_row = left_calculation_row + NUM_ROWS_TO_START_CALCULATION - 1;
            }
            else
            {
                should_send = false;
                has_sent_all = true;
            }
        }

        matrix = matrix_new;

        //std::cout << "Iteration: " << current_iteration + 1 << std::endl
        //          << matrix_new << std::endl << std::endl;
    }
}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Bad parameters" << std::endl;
        exit(EXIT_FAILURE);
    }

    bool is_seidel = false;
    if (strncmp(argv[1], SEIDEL_PAR_STRING, PAR_STRING_SIZE) == 0)
    {
        is_seidel = true;
    }
    else if (strncmp(argv[1], JACOBI_PAR_STRING, PAR_STRING_SIZE) == 0)
    {
        is_seidel = false;
    }
    else
    {
        std::cerr << "Bad parameters" << std::endl;
        exit(EXIT_FAILURE);
    }
    

    MatrixType matrix(LEFT_TEMPERATURE, UP_TEMPERATURE, RIGHT_TEMPERATURE, DOWN_TEMPERATURE);
    MatrixType matrix_new(LEFT_TEMPERATURE, UP_TEMPERATURE, RIGHT_TEMPERATURE, DOWN_TEMPERATURE);

    int rank = 0;
    int num_procs = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    const std::size_t num_iterations = 100;

    if (is_seidel)
        Calculator::calculate(matrix, matrix_new, Calculator::calculate_seidel, rank, num_procs, num_iterations);
    else
        Calculator::calculate(matrix, matrix_new, Calculator::calculate_jacobi, rank, num_procs, num_iterations);        

    if (static_cast<std::size_t>(num_procs - rank - 1) == num_iterations % num_procs)
        std::cout << matrix_new << std::endl; 

    MPI_Finalize();

    return EXIT_SUCCESS;
}