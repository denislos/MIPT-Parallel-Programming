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
#define POSITION_INPUT_NUM_ELEMENTS 2
int check_positive_passed_number(long number) { return number >= 0; }


typedef long double fp_type;


/**
 * Calculatation levels (default configuration)
 * lvl 0: number_elements (from arguments)
 * lvl 1: 20.000 elements
*/


// lvl 1
#define L1_NUM_ELEMS 20000ull
#define L1_NUM_SPECIAL_ELEMS 100ull


static const size_t exp_hierarchical_structure_config_num_elems[] = 
{
    L1_NUM_ELEMS
};

static const size_t NUM_CALC_LEVELS = sizeof(exp_hierarchical_structure_config_num_elems) / sizeof(exp_hierarchical_structure_config_num_elems[0]);


int main(int argc, char** argv)
{
    if (argc != 3)
        raise_error("Invalid format");

    const size_t N = (size_t)get_number_from_arguments(argc, argv, POSITION_INPUT_NUM_THREADS, check_positive_passed_number);
    const size_t num_elements = (size_t)get_number_from_arguments(argc, argv, POSITION_INPUT_NUM_ELEMENTS, check_positive_passed_number);

    omp_set_num_threads((int)N);

    size_t level_idx = 0;

    // init exp_hierarchical_structure
    fp_type* exp_hierarchical_structure[NUM_CALC_LEVELS];
    fp_type factorial_tmp_values[exp_hierarchical_structure_config_num_elems[0]];

    for (level_idx = 0; level_idx < NUM_CALC_LEVELS; level_idx++)
    {
        exp_hierarchical_structure[level_idx] = (fp_type*)calloc(exp_hierarchical_structure_config_num_elems[level_idx], sizeof(fp_type));
        
        if (exp_hierarchical_structure[level_idx] == NULL)
            raise_error("Bad calloc");
    }

    // take 1 / 0! into account
    exp_hierarchical_structure[0][0] = 1.0;

    size_t j = 0;

    size_t default_chunk_size = 0;
    size_t num_increased_chunks = 0;
    size_t num_special_chunks = L1_NUM_SPECIAL_ELEMS;

    if (num_elements > L1_NUM_ELEMS)
    {
        default_chunk_size = (num_elements - L1_NUM_SPECIAL_ELEMS) / (L1_NUM_ELEMS - L1_NUM_SPECIAL_ELEMS);
        num_increased_chunks = (num_elements - L1_NUM_SPECIAL_ELEMS) - default_chunk_size * (L1_NUM_ELEMS - L1_NUM_SPECIAL_ELEMS);
    }
    else if (num_elements >= L1_NUM_SPECIAL_ELEMS)
    {
        num_increased_chunks = num_elements - L1_NUM_SPECIAL_ELEMS;
    }
    else
        num_special_chunks = num_elements;

    // main part: lvl 1 calculatation
    #pragma omp parallel for schedule (dynamic)
    for (j = 0; j < L1_NUM_ELEMS; j++)
    {
        const int is_special_chunk = j < num_special_chunks;

        size_t chunk_size = 1;
        size_t current_chunk_base = 1 + j;

        if (!is_special_chunk)
        {
            const int has_increased_chunk = j < (num_increased_chunks + num_special_chunks);

            chunk_size = default_chunk_size + (has_increased_chunk ? 1 : 0);
            current_chunk_base = 1 + num_special_chunks +
                                 (j - num_special_chunks) * default_chunk_size + 
                                 (has_increased_chunk ? (j - num_special_chunks) : num_increased_chunks);
        }

        size_t i = 0;
        fp_type product = 1.0;

        for (i = current_chunk_base; i < current_chunk_base + chunk_size; i++)
        {
            product *= i;
            exp_hierarchical_structure[0][j] += 1 / (product);
        }

        if (j != L1_NUM_ELEMS -1)
            factorial_tmp_values[j + 1] = 1 / (product);
    }

    fp_type product = 1.0;

    for (j = 1; j < L1_NUM_ELEMS; j++)
    {
        if (default_chunk_size != 0 || j < (num_special_chunks + num_increased_chunks))
        {
            product *= factorial_tmp_values[j];
            exp_hierarchical_structure[0][j] *= product;
        }
    }

    fp_type final_sum_value = 0;
    fp_type extra_sum_value = 0;

    for (j = L1_NUM_SPECIAL_ELEMS; j > 0; j--)
        final_sum_value += exp_hierarchical_structure[NUM_CALC_LEVELS - 1][j - 1];

    for (j = exp_hierarchical_structure_config_num_elems[NUM_CALC_LEVELS - 1]; j >= L1_NUM_SPECIAL_ELEMS; j--)
        extra_sum_value += exp_hierarchical_structure[NUM_CALC_LEVELS - 1][j - 1];

    final_sum_value += extra_sum_value;

    printf("Exponent: %.20Lf\n", final_sum_value);

    return EXIT_SUCCESS;
}
