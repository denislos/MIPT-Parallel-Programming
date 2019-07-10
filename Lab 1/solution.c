

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "tools.h"
#include "problem_config.h"



unsigned long get_t_num_steps_from_arguments(int argc, char** argv);
unsigned long get_x_num_steps_from_arguments(int argc, char** argv);

long get_number_from_arguments(char** argv, unsigned int pos);





int main(int argc, char** argv)
{
    const unsigned long x_num_steps = get_x_num_steps_from_arguments(argc, argv);
    const unsigned long t_num_steps = get_t_num_steps_from_arguments(argc, argv);

    const double time_step = (double)T_MAX / t_num_steps;
    const double x_step    = (double)X_MAX / x_num_steps;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        raise_error("Bad MPI_Init");


    int num_procs = 0;
    int rank = 0;

    unsigned long i = 0;

    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
        raise_error("Bad MPI_Comm_rank");

    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        raise_error("Bad MPI_Comm_size");

    if ((unsigned long)num_procs > x_num_steps)
        raise_error("Bad configuration");

    // Init matrix
    double** matrix = (double**)calloc(t_num_steps, sizeof(double**));
    if (matrix == NULL)
        raise_error("Bad alloc");

    for (i = 0; i < t_num_steps; i++)
    {
        matrix[i] = (double*)calloc(x_num_steps, sizeof(double*));
        if (matrix[i] == NULL)
            raise_error("Bad alloc");
    }

    for (i = 0; i < t_num_steps; i++)
    {
        matrix[i][0] = psi(time_step * i);
    }

    for (i = 0; i < x_num_steps; i++)
    {
        matrix[0][i] = phi(x_step * i);
    }


    const unsigned long granulation = x_num_steps / (unsigned long)num_procs;
    unsigned long n_current = granulation;

    if (x_num_steps % (unsigned long)num_procs != 0 && rank == num_procs - 1)
    {
        n_current = x_num_steps - granulation * (num_procs - 1);
    }

    const unsigned long n_start  = granulation * rank;
    const unsigned long n_finish = n_start + n_current - 1;

    //fprintf(stderr, "rank %d n_start %ld n_finish %ld\n", rank, n_start, n_finish);

    for (i = 1; i < x_num_steps; i++)
    {
        matrix[1][i] = matrix[0][i] + time_step * (f(x_step * i, 0) - (matrix[0][i] - matrix[0][i-1]) / x_step);
    }

    unsigned long t = 1;

    double start_wtime = MPI_Wtime();

    for (t = 1; t < t_num_steps - 1; t++)
    {
        for (i = (n_start == 0) ? 1 : n_start; i <= n_finish; i++)
        {
            if (i == x_num_steps - 1)
            {
                matrix[t+1][i] = matrix[t][i] + time_step * (f(x_step * i, time_step * i) - (matrix[t][i] - matrix[t][i-1]) / x_step);
            }
            else
            {
                matrix[t+1][i] = matrix[t-1][i] + time_step * (2 * f(x_step * i, time_step * i) - (matrix[t][i+1] - matrix[t][i-1]) / x_step);
            }
        }

        if (num_procs == 1)
            continue;
        
        if (rank == 0)
        {
            MPI_Send(&matrix[t+1][n_finish], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&matrix[t+1][n_finish + 1], 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (rank == num_procs - 1)
        {
            MPI_Recv(&matrix[t+1][n_start - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&matrix[t+1][n_start], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        }
        else if (rank % 2 == 0)
        {
            MPI_Send(&matrix[t+1][n_finish], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&matrix[t+1][n_finish + 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(&matrix[t+1][n_start - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&matrix[t+1][n_start], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&matrix[t+1][n_start - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&matrix[t+1][n_start], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);

            MPI_Send(&matrix[t+1][n_finish], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&matrix[t+1][n_finish + 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }


    if (rank == 0)
    {
        double finish_wtime = MPI_Wtime();

        printf("\nWorking time: %lg\n", finish_wtime - start_wtime);
    }

    
    if (rank == 0)
    {
        for (int n_proc = 1; n_proc < num_procs - 1; n_proc++)
        {
            for (t = 1; t < t_num_steps; t++)
            {
                MPI_Recv(&matrix[t][granulation * n_proc], granulation, MPI_DOUBLE, n_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        if (num_procs != 1)
        {
            for (t = 1; t < t_num_steps; t++)
            {
                MPI_Recv(&matrix[t][granulation * (num_procs - 1)], 
                         x_num_steps - granulation * (num_procs - 1),
                         MPI_DOUBLE, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
    else
    {
        for (t = 1; t < t_num_steps; t++)
        {
            MPI_Send(&matrix[t][n_start], n_finish - n_start + 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0)
    {
        for (t = 0; t < t_num_steps; t++)
        {
            for (i = 0; i < x_num_steps; i++)
            {
                printf("%lg ", matrix[t][i]);
            }

            printf("\n");
        }
    }

    if (MPI_Finalize() != MPI_SUCCESS)
        raise_error("Bad MPI_Finalize");

    for (i = 0; i < t_num_steps; i++)
    {
        free(&matrix[i][0]);
    }

    free(matrix);

    return EXIT_SUCCESS;
}






unsigned long get_x_num_steps_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGUMENTS)
        raise_error("Usage [x_num_steps] [t_num_steps]");

    const long val = get_number_from_arguments(argv, 1);

    if (val <= 0)
        raise_error("x_num_steps should be a positive number");

    return (unsigned long)val;
}


unsigned long get_t_num_steps_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGUMENTS)
        raise_error("Usage [x_num_steps] [t_num_steps]");

    const long val = get_number_from_arguments(argv, 2);
    
    if (val <= 0)
        raise_error("t_num_steps should be a positive number");

    return (unsigned long)val;
}


long get_number_from_arguments(char** argv, unsigned int pos)
{
    const int base = 10;
    char *endptr, *str;
    long val;

    str = argv[pos];

    errno = 0;
    val = strtol(str, &endptr, base);

    /* Check for various possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0))
    {
        raise_error("Bad strtol");
    }

    if (endptr == str)
        raise_error("No digits were found in N");

    if (*endptr != '\0')
        raise_error("Further characters after N");

    return val;
}