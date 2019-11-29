#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mpi.h>


#define ISIZE 10
#define JSIZE 10




int main(int argc, char **argv)
{
    double a[ISIZE][JSIZE];

    if (ISIZE < 3 || JSIZE < 2)
    {
        perror("Not supported\n");
        exit(1);
    }

    double matrix[3 * JSIZE + 2 * (ISIZE - 3)][JSIZE / 2 + 1];
    int elems[3 * JSIZE + 2 * (ISIZE - 3)];

    int i = 0, j = 0;
    FILE* ff = NULL;


    for (i=0; i<ISIZE; i++)
    {
        for (j=0; j<JSIZE; j++)
        {
            a[i][j] = 10*i +j;
        }
    }

    const double start_time = MPI_Wtime();

    int idx_i = 0;
    int idx_j = 0;
    int count = 0;

    for (i = 0; i < 3; i++)
        for (j = 0; j < JSIZE; j++)
        {
            idx_i = i;
            idx_j = j;
            count = 0;

            do {
                matrix[i * JSIZE + j][count] = a[idx_i][idx_j];

                idx_i += 3;
                idx_j -= 2;

                count++;

            } while(idx_i < ISIZE && idx_j >= 0);

            elems[i * JSIZE + j] = count;
        }

    for (i = 3; i < ISIZE; i++)
        for (j = JSIZE - 2; j < JSIZE; j++)
        {
            idx_i = i;
            idx_j = j;
            count = 0;

            do {
               matrix[3 * JSIZE + 2 * (i - 3) + (j - JSIZE - 2)][count] = a[idx_i][idx_j];
               
               idx_i += 3;
               idx_j -= 2;

               count++;

            } while(idx_i < ISIZE && idx_j >= 0);

            elems[3 * JSIZE + 2 * (i - 3) + (j - JSIZE - 2)] = count;
        }

    int rank = 0;
    int num_procs = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


    const int num_elems = 3 * JSIZE + 2 * (ISIZE - 3);
    const int group_size = JSIZE / 2 + 1;

    const int granulation = num_elems / num_procs;
    const int n_current = (num_elems % num_procs != 0 && rank == num_procs - 1) ? (num_elems - granulation * (num_procs - 1)) : granulation;

    const int n_start  = granulation * rank;
    const int n_finish = n_start + n_current - 1;

    for (i = n_start; i <= n_finish; i++)
    {
        for (j = 1; j < elems[i]; j++)
        {
            matrix[i][j] = sin(0.00001 * matrix[i][j - 1]);
        }
    }

    
    double s_time = MPI_Wtime();

    if (num_procs != 1)
    {
        if (rank == 0)
        {
            for (i = 1; i < num_procs - 1; i++)
                MPI_Recv(&matrix[granulation * i], granulation * group_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
            MPI_Recv(&matrix[granulation * (num_procs - 1)], 
                     (num_elems  - granulation * (num_procs - 1)) * group_size, 
                     MPI_DOUBLE, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
        }
        else
        {
            MPI_Send(&matrix[n_start], (n_finish - n_start + 1) * group_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    double f_time = MPI_Wtime();
    
    if (rank == 0)
    {
        for (i = 0; i < 3; i++)
            for (j = 0; j < JSIZE; j++)
            {
                idx_i = i;
                idx_j = j;
                count = 0;

                do {
                    a[idx_i][idx_j] = matrix[i * JSIZE + j][count];

                    idx_i += 3;
                    idx_j -= 2;
                    count++;

                } while(idx_i < ISIZE && idx_j >= 0);
            }

        for (i = 3; i < ISIZE; i++)
            for (j = JSIZE - 2; j < JSIZE; j++)
            {
                idx_i = i;
                idx_j = j;
                count = 0;

                do {
                    a[idx_i][idx_j] = matrix[3 * JSIZE + 2 * (i - 3) + (j - JSIZE - 2)][/*i - 1 + */count];
               
                    idx_i += 3;
                    idx_j -= 2;
                    count++;

                } while(idx_i < ISIZE && idx_j >= 0);
            }
    }

    const double end_time = MPI_Wtime();

    if (rank == 0)
        printf("Calculation time: %lg %lg\n", end_time - start_time, f_time - s_time);


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
                fprintf(ff,"%.10f ",a[i][j]);
        
            fprintf(ff,"\n");
        }

        fclose(ff);
    }


    MPI_Finalize();

    return EXIT_SUCCESS;
}