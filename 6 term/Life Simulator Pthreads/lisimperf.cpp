

#include <exception>
#include <string>
#include <iostream>
#include <errno.h>
#include <limits.h>


#include <pthread.h>


#include "tools.h"
#include "config.h"
#include "core_matrix.h"


UnitCoreMatrix core_matrix;


unsigned int get_num_steps_from_arguments(int argc, char** argv);
int get_num_procs_from_arguments(int argc, char** argv);
long get_integer_number_from_arguments(char** argv, unsigned int pos);

void create_core_matrix_from_arguments(int argc, char** argv, UnitCoreMatrix* core_matrix);

struct thread_data_t
{
    unsigned int start_point = 0;
    unsigned int end_point   = 0;
};

void* thread_routine(void* args)
{
    const thread_data_t* data = (thread_data_t*)args;

    const unsigned int start_point = data->start_point;
    const unsigned int end_point   = data->end_point;

    core_matrix.clock(true, start_point, end_point);

    return NULL;
}


int main(int argc, char** argv)
{
    const unsigned int total_num_steps = get_num_steps_from_arguments(argc, argv);
    int num_procs = get_num_procs_from_arguments(argc, argv);

    create_core_matrix_from_arguments(argc, argv, &core_matrix);

    if ((unsigned int)num_procs > CORE_MATRIX_SIZE)
        num_procs = CORE_MATRIX_SIZE;


    // Find a right number of elements for each process
    const unsigned int granulation = CORE_MATRIX_SIZE / (unsigned int)num_procs;

    pthread_t* threads = new pthread_t[num_procs];
    thread_data_t* data_array = new thread_data_t[num_procs];

    for (int rank = 0; rank < num_procs; rank++)
    {
        unsigned int n_current = granulation;

        if (CORE_MATRIX_SIZE % (unsigned int)num_procs != 0 && rank == num_procs - 1)
        {
            n_current = CORE_MATRIX_SIZE - granulation * (num_procs - 1);
        }

        const unsigned int n_start  = granulation * rank;
        const unsigned int n_finish = n_start + n_current - 1;

        data_array[rank].start_point = n_start;
        data_array[rank].end_point   = n_finish;
    }

    do 
    {
        for (int rank = 0; rank < num_procs; rank++)
        {
            if (pthread_create(&threads[rank], NULL, thread_routine, &data_array[rank]) == -1)
                raise_error("Bad pthread create");
        }

        for (int rank = 0; rank < num_procs; rank++)
        {
            if (pthread_join(threads[rank], NULL) == -1)
                raise_error("Bad pthread join");
        }
        
        core_matrix.update_clock();

    } while (core_matrix.get_time_step() < total_num_steps);

    core_matrix.print();

    delete threads;
    delete data_array;

    return EXIT_SUCCESS;
}


unsigned int get_num_steps_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGS)
        raise_error("Usage [num_steps] [init_file]");
    
    int num_steps = 0;

    try
    {
        num_steps = std::stoi(argv[1]);
    }
    catch (std::exception& e)
    {
        raise_error(e.what());
    }

    if (num_steps <= 0)
        raise_error("Incorrect number of steps");

    return (unsigned int)num_steps; 
}


void create_core_matrix_from_arguments(int argc, char** argv, UnitCoreMatrix* core_matrix)
{
    if (argc != EXPECTED_NUM_ARGS)
        raise_error("Usage [num_steps] [init_file]");

    core_matrix->init_from_file(std::string(argv[2]));
}

int get_num_procs_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGS)
        raise_error("Usage [num_procs] [precision]");

    const long val = get_integer_number_from_arguments(argv, 3);
    
    if (val <= 0)
        raise_error("num_procs should be a positive number");

    return (int)val;
}


long get_integer_number_from_arguments(char** argv, unsigned int pos)
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