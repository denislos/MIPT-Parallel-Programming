

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>


int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    double x = 0;

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        double start_wtime = MPI_Wtime();
        MPI_Recv(&x, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        double finish_wtime = MPI_Wtime();

        printf("Transfer time: %lg\n", finish_wtime - start_wtime);
    }
    else
    {
        MPI_Send(&x, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    

    MPI_Finalize();

    return EXIT_SUCCESS;
}