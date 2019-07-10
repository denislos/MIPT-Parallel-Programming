

#include <fstream>
#include <string>

#include <pthread.h>

#include "tools.h"
#include "core_matrix.h"


pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;


void UnitCoreMatrix::init_from_file(const std::string& init_filename)
{
    std::ifstream init_file(init_filename, std::fstream::in);

    if (!init_file.good())
        raise_error("Error opening an init file");

    char ch = ' ';

    for (unsigned int i = 0; i < CORE_MATRIX_SIZE; i++)
    {
        for (unsigned int j = 0; j < CORE_MATRIX_SIZE; j++)
        {
            if (!init_file.good())
                raise_error("Error reading an init file");

            init_file >> ch;

            if (ch == '0')
            { 
               matrix[i][j].is_alive_even = false;
            }
            else if (ch == '1')
            {
                matrix[i][j].is_alive_even = true;
            }
            else
            {
                std::cerr << "Symbol: " << (int)ch << std::endl;
                raise_error("Incorrect symbol encountered in the init file");
            }
        }
    }

    init_file.close();
}


void UnitCoreMatrix::clock(bool use_global_points, unsigned int start_pos, unsigned int end_pos)
{
    for (unsigned int line_pos = ((!use_global_points) ? line_start: start_pos); line_pos <= ((!use_global_points) ? line_end : end_pos); line_pos++)
    {
        for (unsigned int pos = 0; pos < CORE_MATRIX_SIZE; pos++)
        {
            unsigned int current_status = 0;

            if (line_pos != 0)
            {
                if (pos != 0)
                    current_status += get_entry_status(line_pos - 1, pos - 1);
                
                current_status += get_entry_status(line_pos - 1, pos);

                if (pos != CORE_MATRIX_SIZE - 1)
                    current_status += get_entry_status(line_pos - 1, pos + 1);
            }

            if (pos != 0)
                current_status += get_entry_status(line_pos, pos - 1);

            if (pos != CORE_MATRIX_SIZE - 1)
                current_status += get_entry_status(line_pos, pos + 1);

            if (line_pos != CORE_MATRIX_SIZE - 1)
            {
                if (pos != 0)
                    current_status += get_entry_status(line_pos + 1, pos - 1);

                current_status += get_entry_status(line_pos + 1, pos);

                if (pos != CORE_MATRIX_SIZE - 1)
                    current_status += get_entry_status(line_pos + 1, pos + 1);
            }


            pthread_mutex_lock(&global_mutex);

            if (current_status >= DEATH_THRESHOLD)
            {
                set_entry_status(line_pos, pos, false);
            }
            else if (current_status > 0)
            {
                set_entry_status(line_pos, pos, true);
            }
            else if (current_status == 0 && get_entry_status(line_pos, pos))
            {
                set_entry_status(line_pos, pos, true);
            }

            pthread_mutex_unlock(&global_mutex);
        }
    }
}


void UnitCoreMatrix::print() const
{
    for (unsigned int i = 0; i < CORE_MATRIX_SIZE; i++)
    {
        for (unsigned int j = 0; j < CORE_MATRIX_SIZE; j++)
        {
            std::cout << get_entry_status(i, j);
        }

        std::cout << std::endl;
    }
}


void UnitCoreMatrix::print(const std::string& output_filename) const
{
    std::ofstream output_file(output_filename, std::fstream::out);

    if (!output_file.good())
        raise_error("Error opening an output file");

    for (unsigned int i = 0; i < CORE_MATRIX_SIZE; i++)
    {
        for (unsigned int j = 0; j < CORE_MATRIX_SIZE; j++)
        {
            output_file << get_entry_status(i, j);
        }

        output_file << std::endl;
    }

    output_file.close();
}