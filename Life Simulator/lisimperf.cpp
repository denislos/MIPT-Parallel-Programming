

#include <exception>
#include <string>
#include <iostream>


#include <mpi.h>


#include "tools.h"
#include "config.h"
#include "core_matrix.h"


unsigned int get_num_steps_from_arguments(int argc, char** argv);
void create_core_matrix_from_arguments(int argc, char** argv, UnitCoreMatrix* core_matrix);


MPI_Datatype MPI_Entry;


int main(int argc, char** argv)
{
    const unsigned int total_num_steps = get_num_steps_from_arguments(argc, argv);

    UnitCoreMatrix core_matrix;
    create_core_matrix_from_arguments(argc, argv, &core_matrix);

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
        raise_error("Bad MPI_Init");

    int mpi_entry_barray[2] = {1, 1};
    MPI_Aint mpi_entry_disp_array[2] = {offsetof(Entry, is_alive_even), offsetof(Entry, is_alive_odd)};
    MPI_Datatype mpi_entry_types[2] = {MPI_C_BOOL, MPI_C_BOOL};

    MPI_Type_create_struct(2, mpi_entry_barray, mpi_entry_disp_array, mpi_entry_types, &MPI_Entry);
    MPI_Type_commit(&MPI_Entry);

    int rank = 0;
    int num_procs = 0;

    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
        raise_error("Bad MPI_Comm_rank");

    if (MPI_Comm_size(MPI_COMM_WORLD, &num_procs) != MPI_SUCCESS)
        raise_error("Bad MPI_Comm_size");

    if ((unsigned int)num_procs > CORE_MATRIX_SIZE)
        num_procs = CORE_MATRIX_SIZE;


    // Find a right number of elements for each process
    const unsigned int granulation = CORE_MATRIX_SIZE / (unsigned int)num_procs;
    unsigned int n_current = granulation;

    if (CORE_MATRIX_SIZE % (unsigned int)num_procs != 0 && rank == num_procs - 1)
    {
        n_current = CORE_MATRIX_SIZE - granulation * (num_procs - 1);
    }

    const unsigned int n_start  = granulation * rank;
    const unsigned int n_finish = n_start + n_current - 1;

    core_matrix.set_line_start(n_start);
    core_matrix.set_line_end(n_finish);

    double start_wtime = MPI_Wtime();

    do 
    {
        core_matrix.clock();

        if (num_procs == 1)
            continue;
        
        if (rank == 0)
        {
            MPI_Send(core_matrix.get_line_pointer(n_finish), CORE_MATRIX_SIZE, MPI_Entry, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(core_matrix.get_line_pointer(n_finish + 1), CORE_MATRIX_SIZE, MPI_Entry, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (rank == num_procs - 1)
        {
            MPI_Recv(core_matrix.get_line_pointer(n_start - 1), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(core_matrix.get_line_pointer(n_start), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD);
        }
        else if (rank % 2 == 0)
        {
            MPI_Send(core_matrix.get_line_pointer(n_finish), CORE_MATRIX_SIZE, MPI_Entry, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(core_matrix.get_line_pointer(n_finish + 1), CORE_MATRIX_SIZE, MPI_Entry, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(core_matrix.get_line_pointer(n_start - 1), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(core_matrix.get_line_pointer(n_start), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(core_matrix.get_line_pointer(n_start - 1), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(core_matrix.get_line_pointer(n_start), CORE_MATRIX_SIZE, MPI_Entry, rank - 1, 0, MPI_COMM_WORLD);

            MPI_Send(core_matrix.get_line_pointer(n_finish), CORE_MATRIX_SIZE, MPI_Entry, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(core_matrix.get_line_pointer(n_finish + 1), CORE_MATRIX_SIZE, MPI_Entry, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

    } while (core_matrix.get_time_step() < total_num_steps);

    if (rank == 0)
    {
        for (int n_proc = 1; n_proc < num_procs - 1; n_proc++)
        {
            MPI_Recv(core_matrix.get_line_pointer(granulation * n_proc), CORE_MATRIX_SIZE * granulation, MPI_Entry, n_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (num_procs != 1)
        {
            MPI_Recv(core_matrix.get_line_pointer(granulation * (num_procs - 1)), 
                     CORE_MATRIX_SIZE * (CORE_MATRIX_SIZE - granulation * (num_procs - 1)),
                     MPI_Entry, num_procs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        core_matrix.set_line_start(0);
        core_matrix.set_line_end(CORE_MATRIX_SIZE - 1);
    }
    else
    {
        MPI_Send(core_matrix.get_line_pointer(n_start), CORE_MATRIX_SIZE * (n_finish - n_start + 1), MPI_Entry, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
        double finish_wtime = MPI_Wtime();

        core_matrix.print();

        std::cout << std::endl << "Working time: " << finish_wtime - start_wtime << std::endl;
    }

    if (MPI_Finalize() != MPI_SUCCESS)
        raise_error("Bad MPI_Finalize");

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