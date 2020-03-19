
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <mpi.h>

#include "support.h"


#define N 10000

#define PI 3.14

#define START_POINT 0
#define FINISH_POINT 3 * PI

#define START_TARGET 0
#define FINISH_TARGET -2

static inline double f(double t)
{
    return sin(t);
}


static const char* const output_shooting_file = "shooting_data.dat";
static const char* const output_final_file    = "final_data.dat";


int main(int argc, char** argv)
{
    double* shooting_array   = NULL;
    double* shooting_array_w = NULL;
    double* final_array      = NULL;

    double* velocities  = NULL;
    double* coordinates = NULL;

    int rank = 0;
    int num_procs = 0;

    if (MPI_Init(&argc, &argv) == -1)
        raise_error("Bad MPI_Init");
    
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) == -1)
        raise_error("Bad MPI_Comm_rank");
    
    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) == -1)
        raise_error("Bad MPI_Comm_size");



    if (num_procs < 1)
        raise_error("Bad configuration");

    final_array = (double*)malloc(N * sizeof(*final_array));
    if (final_array == NULL)
        raise_error("Bad malloc");

    coordinates = (double*)malloc((num_procs + 1) * sizeof(*coordinates));
    velocities  = (double*)malloc((num_procs + 1) * sizeof(*velocities));

    if (!(coordinates || velocities))
        raise_error("Bad malloc");

    const int granulation = N / num_procs;
    const int n_current = (N % num_procs != 0 && rank == num_procs - 1) 
                          ? (N - granulation * (num_procs - 1)) 
                          : granulation;

    const int n_start  = granulation * rank;
    const int n_finish = n_start + n_current - 1;

    const double step = (FINISH_POINT - START_POINT) / N;

    const double point_start  = n_start * step;

    if (rank == 0)
    {
        shooting_array = (double*)malloc(N * sizeof(*shooting_array));
    }
    else
    {
        shooting_array = (double*)malloc(n_current * sizeof(*shooting_array));
    }
    
    shooting_array_w = (double*)malloc(n_current * sizeof(*shooting_array_w));
    
    if (shooting_array == NULL || shooting_array_w == NULL)
        raise_error("Bad malloc"); 

    int i = 0;
    
    const double start_time = MPI_Wtime();

    /*** SHOOTING ***/

    shooting_array[0]   = (rank == 0) ? START_TARGET : 0;
    shooting_array_w[0] = 0;

    for (i = 0;  i < n_current - 1; i++)
    {
        shooting_array[i + 1]   = shooting_array[i]   + step * shooting_array_w[i];
        shooting_array_w[i + 1] = shooting_array_w[i] + step * f(point_start + step * i);
    }

    const double adjustment = shooting_array[n_current - 1];
    const double velocity   = shooting_array_w[n_current - 1];

    coordinates[0] = START_TARGET;
    velocities[0]  = 0;

    for (i = rank; i < num_procs; i++)
    {
        coordinates[i + 1] = adjustment;
        velocities[i + 1]  = velocity;
    }

    if (rank == 0)
        MPI_Reduce(MPI_IN_PLACE, coordinates + 1, num_procs, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    else
        MPI_Reduce(coordinates + 1, coordinates + 1, num_procs, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        MPI_Reduce(MPI_IN_PLACE, velocities + 1, num_procs, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    else
        MPI_Reduce(velocities + 1, velocities + 1, num_procs, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double start_velocity = 0;
    double current_velocity = 0;

    if (rank == 0)
    {
        double sum = 0;

        for (i = 1; i < num_procs; i++)
        {
            coordinates[i] += sum;
            sum += velocities[i] * step * granulation; 
        }

        const double final_adjustment = coordinates[num_procs] + sum;
        start_velocity = (FINISH_TARGET - final_adjustment) / (FINISH_POINT - START_POINT);

        for (i = 1; i < num_procs; i++)
        {
            coordinates[i] += i * start_velocity * step * granulation;
        }

        for (i = 1; i < num_procs; i++)
        {
            double tmp_velocity = start_velocity + velocities[i];

            MPI_Send(&tmp_velocity, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(&coordinates[i], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }

        current_velocity = start_velocity;
    }
    else
    {
        MPI_Recv(&current_velocity, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&coordinates[rank], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    /*** ACTUAL COMPUTATION ***/
    final_array[n_start] = coordinates[rank];
    shooting_array_w[0]  = current_velocity;

    for (i = n_start;  i < n_finish; i++)
    {
        final_array[i + 1]      = final_array[i]      + step * shooting_array_w[i - n_start];
        shooting_array_w[i + 1 - n_start] = shooting_array_w[i - n_start] + step * f(point_start + step * (i - n_start));
    }

    if (num_procs != 1)
    {
        if (rank == 0)
        {
            for (i = 1; i < num_procs - 1; i++)
                MPI_Recv(&final_array[granulation * i], granulation, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
            MPI_Recv(&final_array[granulation * (num_procs - 1)], 
                     (N  - granulation * (num_procs - 1)), 
                     MPI_DOUBLE, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
        }
        else
        {
            MPI_Send(&final_array[n_start], (n_finish - n_start + 1), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    const double end_time = MPI_Wtime();

    /*** OUTPUT ***/
    if (num_procs != 1)
    {
        if (rank == 0)
        {
            for (i = 1; i < num_procs - 1; i++)
                MPI_Recv(&shooting_array[granulation * i], granulation, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
            MPI_Recv(&shooting_array[granulation * (num_procs - 1)], 
                     (N  - granulation * (num_procs - 1)), 
                     MPI_DOUBLE, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
        }
        else
        {
            MPI_Send(shooting_array, n_current, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0)
    {
        printf("Working time: %lg\n", end_time - start_time);

        // Shooting data
        FILE* output_file = fopen(output_shooting_file, "w");
        if (output_file == NULL)
            raise_error("Bad fopen");

        for (i = 0; i < N; i++)
        {
            fprintf(output_file, "%lg %lg\n", START_POINT + i * step, shooting_array[i]);
        }

        fclose(output_file);

        // Final data
        output_file = fopen(output_final_file, "w");
        if (output_file == NULL)
            raise_error("Bad fopen");

        for (i = 0; i < N; i++)
        {
            fprintf(output_file, "%lg %lg\n", START_POINT + i * step, final_array[i]);
        }

        fclose(output_file);
    }   

    free(shooting_array);
    free(shooting_array_w);
    free(final_array);

    free(coordinates);
    free(velocities);

    MPI_Finalize();

    return EXIT_SUCCESS;
}