#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include<limits.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "support.h"

#include <omp.h>


#define POSITION_INPUT_NUM_THREADS 1
int check_passed_number_procs(long number) { return number >= 0; }



/**
 * Calculatation levels (default configuration)
 * lvl 0: 1.000.000.000 elements
 * lvl 1: 20.000 elements (50k - L1_CHUNK_SIZE)
 * lvl 2: 100 elements (100 - L2_CHUNK_SIZE)
 * lvl 3: 5 elements (20 - L3_CHUNK_SIZE)
 * lvl 4: s[0] + s[1] + s[2] + s[3] + s[4]
*/

// lvl 0
#define L0_NUM_ELEMS 1000000000ull

// lvl 1
#define L1_NUM_ELEMS 20000ull
#define L1_CHUNK_SIZE (L0_NUM_ELEMS / L1_NUM_ELEMS)

// lvl 2
#define L2_NUM_ELEMS 100ull

// lvl 3
#define L3_NUM_ELEMS 5ull


static const size_t sum_hierarchical_structure_config_num_elems[] = 
{
    L1_NUM_ELEMS,
    L2_NUM_ELEMS,
    L3_NUM_ELEMS
};


static const size_t sum_hierarchical_structure_config_chunk_sizes[] = 
{
    L0_NUM_ELEMS / L1_NUM_ELEMS,
    L1_NUM_ELEMS / L2_NUM_ELEMS,
    L2_NUM_ELEMS / L3_NUM_ELEMS
};


static const size_t NUM_CALC_LEVELS = sizeof(sum_hierarchical_structure_config_num_elems) / sizeof(sum_hierarchical_structure_config_num_elems[0]);


int main(int argc, char** argv)
{
    if (argc != 2)
        raise_error("Invalid format");

    const size_t N = (size_t)get_number_from_arguments(argc, argv, POSITION_INPUT_NUM_THREADS, check_passed_number_procs);

    omp_set_num_threads((int)N);

    size_t level_idx = 0;

    // init sum_hierarchical_structure
    double* sum_hierarchical_structure[NUM_CALC_LEVELS];

    for (level_idx = 0; level_idx < NUM_CALC_LEVELS; level_idx++)
    {
        sum_hierarchical_structure[level_idx] = (double*)calloc(sum_hierarchical_structure_config_num_elems[level_idx], sizeof(double));
        
        if (sum_hierarchical_structure[level_idx] == NULL)
            raise_error("Bad calloc");
    }

    size_t j = 0;

    // main part: lvl 1 calculatation
    #pragma omp parallel for schedule (dynamic)
    for (j = 0; j < L1_NUM_ELEMS; j++)
    {
        const size_t current_chunk_base = j * L1_CHUNK_SIZE + 1;

        size_t i = 0;

        for (i = 0; i < L1_CHUNK_SIZE; i++)
        {
            sum_hierarchical_structure[0][j] += 1 / (double)(current_chunk_base + i);
        }
    }

    // lvl2 - lvl3 calculations
    for (level_idx = 1; level_idx < NUM_CALC_LEVELS; level_idx++)
    {
        #pragma omp parallel for schedule(dynamic)
        for (j = 0; j < sum_hierarchical_structure_config_num_elems[level_idx]; j++)
        {
            const size_t current_chunk_base = j * sum_hierarchical_structure_config_chunk_sizes[level_idx];

            size_t i = 0;

            for (i = 0; i < sum_hierarchical_structure_config_chunk_sizes[level_idx]; i++)
            {
                sum_hierarchical_structure[level_idx][j] += sum_hierarchical_structure[level_idx - 1][i + current_chunk_base];
            }
        }
   }


    double final_sum_value = 0;

    for (j = 0; j < sum_hierarchical_structure_config_num_elems[NUM_CALC_LEVELS - 1]; j++)
        final_sum_value += sum_hierarchical_structure[NUM_CALC_LEVELS - 1][j];


    printf("Sum of harmonic series: %.20f\n", final_sum_value);


    return EXIT_SUCCESS;
}
