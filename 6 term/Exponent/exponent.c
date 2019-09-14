
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include <mpi.h>
#include <gmp.h>

#include "mpi_gmp.h"



#ifndef ERROR
#define ERROR(description)       \
    do                           \
    {                            \
        perror(description); \
        exit(EXIT_FAILURE);      \
    }                            \
    while(0)

#endif



unsigned long check_for_positive(long number, const char* error_msg);
long get_number_from_argument_string(const char* argument_string);



int main(int argc, char** argv)
{
    // number of elements in series to sum
    unsigned long total_num_elements = 0;

    // number of characters after a point in the output number
    unsigned long precision = 0;

    if (argc != 3)
        ERROR("Usage: [N] [PRECISION]\n");
    else
    {
        total_num_elements = check_for_positive(get_number_from_argument_string(argv[1]), "N should be a positive number");
        precision          = check_for_positive(get_number_from_argument_string(argv[2]), "Precision should be a positive number");

        if (precision > 100000)
            ERROR("Passed PRECISION should not be greater than 100000");
    }

    const unsigned long binary_precision = precision * ceil(log2(10.0));
    mpf_set_default_prec(binary_precision);

    int rank = 0;

    // Init
    int num_procs  = 0;
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        ERROR("Bad MPI_Init\n");


    commit_mpf(&(MPI_MPF), binary_precision, MPI_COMM_WORLD);
    create_mpf_op(&(MPI_MPF_SUM), _mpi_mpf_add, MPI_COMM_WORLD);


    // get a rank of the process
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_rank\n");

    // get a total number of processes
    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        ERROR("Bad MPI_Comm_size\n");
    
    double start_wtime = MPI_Wtime();

    // Find a right number of elements to sum per each process
    const unsigned long granulation = total_num_elements / num_procs;
    unsigned long n_current = granulation;

    if (total_num_elements % (unsigned long)num_procs != 0 && rank == num_procs - 1)
    {
        n_current = total_num_elements - granulation * (num_procs - 1);
    }

    const unsigned long n_start  = granulation * rank;
    const unsigned long n_finish = n_start + n_current - 1;

    mpf_t sum;
    mpf_t current_elem;

    mpf_init(sum); // set a value of the sum to 0
    mpf_init_set_d(current_elem, 1); // set a value of the current elem to 1.0

    unsigned long i = 0;
    for (i = n_start ? n_start : 1; i <= n_finish; i++)
    {
        mpf_div_ui(current_elem, current_elem, i);
        mpf_add(sum, sum, current_elem);
    }

    if (n_start == 0)
        mpf_add_ui(sum, sum, 1);



    // Communication with other processes

    void* packed_mpf = allocbuf_mpf(mpf_get_prec(current_elem), 1);
    if (packed_mpf == NULL)
        ERROR("Bad allocpuf_mpf");

    mpf_t received_elem;
    mpf_init(received_elem);


    if (rank != 0)
    {
        packed_mpf = allocbuf_mpf(mpf_get_prec(current_elem), 1);
        if (packed_mpf == NULL)
            ERROR("Bad allocbuf_mpf");

        if (MPI_Recv(packed_mpf, 1, MPI_MPF, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS)
            ERROR("Bad MPI_Recv\n");

        unpack_mpf(packed_mpf, received_elem, 1);

        mpf_mul(sum, sum, received_elem);
    }

    if (rank != 0 && rank != num_procs - 1)
    {
        mpf_mul(current_elem, current_elem, received_elem);
    }

    if (rank != num_procs - 1)
    {
        // Allocate buffer for the last elem (mpf)
        packed_mpf = allocbuf_mpf(mpf_get_prec(current_elem), 1);
        if (packed_mpf == NULL)
            ERROR("Bad allocbuf_mpf");
    
        // Pack the last elem
        pack_mpf(current_elem, 1, packed_mpf);

        if (MPI_Send(packed_mpf, 1, MPI_MPF, rank + 1, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
            ERROR("Bad MPI_Send\n");
    }


    // Allocate buf for a reduced result
    void* packed_mpf_sum = NULL;

    if (rank == 0)
    {
        packed_mpf_sum = allocbuf_mpf(mpf_get_prec(sum), 1);
        if (packed_mpf_sum == NULL)
            ERROR("Bad allocbuf");
    }

    // Pack a local sum
    pack_mpf(sum, 1, packed_mpf);

    // MPI_Reduce
    if (MPI_Reduce(packed_mpf, packed_mpf_sum, 1, MPI_MPF, MPI_MPF_SUM, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
        ERROR("Bad MPI_Reduce");

    // Unpack and print the final result
    if (rank == 0)
    {
        unpack_mpf(packed_mpf_sum, sum, 1);

        double finish_wtime = MPI_Wtime();

        mpf_out_str(stdout, 10, precision, sum);

        printf("\nWorking time: %lg\n", finish_wtime - start_wtime);
    }

    mpf_clear(sum);
    mpf_clear(current_elem);
    mpf_clear(received_elem);

    free_mpf(&(MPI_MPF));
	free_mpf_op(&(MPI_MPF_SUM));

    // Finalize
    if (MPI_Finalize() != MPI_SUCCESS)
        ERROR("Bad MPI_Finalize\n");

    return EXIT_SUCCESS;
}



long get_number_from_argument_string(const char* str)
{
    const int base = 10;
    char *endptr;
    long val;

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


unsigned long check_for_positive(long number, const char* error_msg)
{
    unsigned long ret_number = 0;

    if (number <= 0)
        ERROR(error_msg);
    else
        ret_number = (unsigned long)number;

    return ret_number;
}
