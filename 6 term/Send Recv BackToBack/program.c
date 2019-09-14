

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>


#ifndef ERROR
#define ERROR(description)   \
    do                       \
    {                        \
        perror(description); \
        exit(EXIT_FAILURE);  \
    } while (0)

#endif
    

#define BUFFER_SIZE 128


static const char* NOTHING_STR = "NOTHING";


int main(int argc, char* argv[])
{
    int rank = 0;
    int num_procs = 0;

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        ERROR("Bad MPI_Init\n");
    
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_rank\n");
    
    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_size\n");


    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    if (rank != 0)
    {
        if (MPI_Recv((void*)buffer, sizeof(buffer), MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS)
            ERROR("Bad MPI_Recv\n");
    }

    if (rank != num_procs - 1)
    {
        sprintf(message, "Hello from %d", rank);

        if (MPI_Send((void*)message, sizeof(message), MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
            ERROR("Bad MPI_Send\n");
    }



    if (num_procs == 1)
        printf("Sorry, I am alone (N = 1)\n");
    else
    {
        printf("PROCESS: %d GOT: %s ...... SENT: %s\n", rank, (rank != 0) ? buffer : NOTHING_STR, (rank != num_procs - 1) ? message : NOTHING_STR);
    }


    if (MPI_Finalize() != MPI_SUCCESS)
        ERROR("Bad MPI_FInalize\n");

    return EXIT_SUCCESS;
}