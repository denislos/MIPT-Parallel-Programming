

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>


#ifndef ERROR
#define ERROR(description)   \
    do                       \
    {                        \
        perror(description); \
        exit(EXIT_FAILURE);  \
    }                        \
    while(0)

#endif


int main(int argc, char** argv)
{
    int current_rank = 0;
    int num_procs = 0;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        ERROR("Bad MPI_Init\n");

    if (MPI_Comm_rank(MPI_COMM_WORLD, &current_rank) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_rank\n");

    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_size\n");

    if (printf("Hello World from process: %d of %d processes\n", current_rank, num_procs) < 0)
        ERROR("Output error in printf\n");

    if (MPI_Finalize() != MPI_SUCCESS)
        ERROR("Bad MPI_Finalize\n");

    return EXIT_SUCCESS;
}