

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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



long get_number_from_arguments(char* argv[]);



int main(int argc, char* argv[])
{
    int rank = 0;
    int num_procs = 0;

    // number of elements of harmonic series
    unsigned long N = 0;


    if (argc < 2)
        ERROR("You should pass a number of elements of harmonic series to use\n");
    else if (argc > 2)
        ERROR("You should pass ONLY a number of elements of harmonic series to use\n");
    else
    {
        long parsed_value = get_number_from_arguments(argv);

        if (parsed_value <= 0)
            ERROR("Number of elements N should be a positive number\n");
        else
            N = (unsigned long)parsed_value;
    }  
    

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        ERROR("Bad MPI_Init\n");


    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_rank\n");

    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_size\n");


    // a number of elements to use in this specific process
    const unsigned long granulation = N / num_procs;
    unsigned long n_current = granulation;
    
    if (N % num_procs != 0 && rank == num_procs - 1)
    {
        n_current = N - granulation * (num_procs - 1);
    }


    // computation
    unsigned long i = 0;
    double result = 0;
    
    const unsigned long i_start  = granulation * rank + 1;
    const unsigned long i_finish = i_start + n_current - 1; 

    for (i = i_start; i <= i_finish; i++)
    {
        result += 1.0 / i;
    }

    //printf("Result: %lg rank:%d num_procs:%d i_start:%ld i_finish:%ld\n", result, rank, num_procs, i_start, i_finish);


    // communication with other computation units
    if (rank == 0)
    {
        double received_data = 0;
        int cnt;

        for (cnt = 1; cnt < num_procs; cnt++)
        {
            if (MPI_Recv((void*)&received_data, 1, MPI_DOUBLE, cnt, 0, 
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS)
            {
                ERROR("Bad MPI_Recv\n");
            }

            result += received_data;
        }

        if (printf("For %ld elements the sum is %lg\n", N, result) < 0)
            ERROR("Bad printf\n");
    }   
    else
    {
        if (MPI_Send((void*)&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
            ERROR("Bad MPI_Send\n");
    }
    

    if (MPI_Finalize() != MPI_SUCCESS)
        ERROR("Bad MPI_Finalize\n");

    return EXIT_SUCCESS;
}




long get_number_from_arguments(char* argv[])
{
    const int base = 10;
    char *endptr, *str;
    long val;

    str = argv[1];

    errno = 0;
    val = strtol(str, &endptr, base);

    /* Check for various possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0))
    {
        ERROR("Bad strtol\n");
    }

    if (endptr == str)
        ERROR("No digits were found in N\n");

    if (*endptr != '\0')
        ERROR("Further characters after N\n");

    return val;
}
