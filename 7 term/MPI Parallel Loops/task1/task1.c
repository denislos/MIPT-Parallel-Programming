#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mpi.h>


#define ISIZE 1000
#define JSIZE 1000




int main(int argc, char **argv)
{
    double a[ISIZE][JSIZE];
    
    int i = 0, j = 0;
    FILE* ff = NULL;


    for (i=0; i<ISIZE; i++)
        for (j=0; j<JSIZE; j++)
            a[i][j] = 10*i +j;

    int rank = 0;
    int num_procs = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


    const double start_time = MPI_Wtime();

    const int granulation = ISIZE / num_procs;
    const int n_current = (ISIZE % num_procs != 0 && rank == num_procs - 1) ? (ISIZE - granulation * (num_procs - 1)) : granulation;

    const int n_start  = granulation * rank;
    const int n_finish = n_start + n_current - 1;

    for (i = n_start; i <= n_finish; i++)
        for (j = 0; j < JSIZE; j++)
            a[i][j] = sin(0.00001*a[i][j]);


    if (num_procs != 1)
    {
    if (rank == 0)
    {
        for (i = 1; i < num_procs - 1; i++)
            MPI_Recv(&a[granulation * i], granulation * JSIZE, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        MPI_Recv(&a[granulation * (num_procs - 1)], 
                 (ISIZE - granulation * (num_procs - 1)) * JSIZE, 
                 MPI_DOUBLE, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
    }
    else
    {
        MPI_Send(&a[n_start], (n_finish - n_start + 1) * JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    }

    const double end_time = MPI_Wtime();

    if (rank == 0)
        printf("Calculation time: %lg\n", end_time - start_time);

    if (rank == 0)
    {
        ff = fopen("result.txt","w");
    
        if (ff == NULL)
        {
            perror("Bad fopen\n");
            exit(EXIT_FAILURE);
        }

        for(i=0; i < ISIZE; i++)
        {
            for (j=0; j < JSIZE; j++)
                fprintf(ff,"%f ",a[i][j]);
        
            fprintf(ff,"\n");
        }

        fclose(ff);
    }


    MPI_Finalize();

    return EXIT_SUCCESS;
}
