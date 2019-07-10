

#ifndef LISIM_CORE_MATRIX_H
#define LISIM_CORE_MATRIX_H


#include <string>


#include "config.h"


struct Entry
{
    bool is_alive_even = false; // for even numbers of time steps
    bool is_alive_odd  = false; // for odd numbers of time steps
};


class UnitCoreMatrix
{
public:
    UnitCoreMatrix() { }

    void clock(bool use_global_points = false, unsigned int start_pos = 0, unsigned int end_pos = 0);

    void init_from_file(const std::string& init_filename);

    void set_line_start(unsigned int n_line) { line_start = n_line; }
    unsigned int get_line_start() const { return line_start; }

    void set_line_end(unsigned int n_line) { line_end = n_line; }
    unsigned int get_line_end() const { return line_end; }

    std::size_t get_time_step() const { return time_step; }

    Entry* get_line_pointer(unsigned int n_line) { return &matrix[n_line][0]; }

    void update_clock() { time_step++; is_even_time_step = !is_even_time_step; }

    void print() const;
    void print(const std::string& output_filename) const;

private:
    Entry matrix[CORE_MATRIX_SIZE][CORE_MATRIX_SIZE];

    unsigned int line_start = 0;
    unsigned int line_end = CORE_MATRIX_SIZE - 1;

    std::size_t time_step = 0;
    bool is_even_time_step = true;

    bool get_entry_status(unsigned int line_pos, unsigned int pos) const
    {
        return (is_even_time_step) ? matrix[line_pos][pos].is_alive_even
                                   : matrix[line_pos][pos].is_alive_odd;
    }

    void set_entry_status(unsigned int line_pos, unsigned int pos, bool is_alive)
    {
        if (is_even_time_step)
            matrix[line_pos][pos].is_alive_odd = is_alive; // for future
        else
            matrix[line_pos][pos].is_alive_even  = is_alive;  // for future
    }
};


#endif // LISIM_CORE_MATRIX_H