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
#define ARRAY_SIZE 16000

int check_passed_number_procs(long number) { return number >= 0; }

int main(int argc, char** argv)
{
    if (argc != 2)
        raise_error("Invalid format");

    const unsigned int N = (unsigned int)get_number_from_arguments(argc, argv, POSITION_INPUT_NUM_THREADS, check_passed_number_procs);

    unsigned int a[ARRAY_SIZE];
    unsigned int i = 0;

    for (i = 0; i < ARRAY_SIZE; i++)
        a[i] = i;

    omp_set_num_threads(N);

    const unsigned int granulation      = (ARRAY_SIZE - 2) / N;
    const unsigned int last_granulation = (((ARRAY_SIZE - 2) % N == 0) ? granulation : ((ARRAY_SIZE - 2) - granulation * N));

    #pragma omp parallel
    {
        const unsigned int id = omp_get_thread_num();

        const unsigned int j_start  = granulation * id;
        const unsigned int j_finish = j_start + ((id != N) ? granulation : last_granulation);

        unsigned int j = 0;
        unsigned prev_middle_value = a[j_start + 1];

        unsigned int first_mean = 0;
        unsigned int last_mean = 0;

        if (j_finish != 0)
        {
            first_mean = (a[j_start] + a[j_start + 1] + a[j_start + 2]) / 3;
            last_mean  = (a[j_finish - 1] + a[j_finish] + a[j_finish + 1]) / 3;
        
            for (j = j_start + 1; j < j_finish - 1; j++)
            {
                const unsigned int tmp = (prev_middle_value + a[j+1] + a[j+2]) / 3;

                prev_middle_value = a[j+1];
                a[j+1] = tmp; 
            }

            #pragma omp barrier

            a[j_start + 1] = first_mean;
            a[j_finish] = last_mean;
        }
    }


    for (i = 0; i < ARRAY_SIZE; i++)
        printf("%d\n", a[i]);

    return EXIT_SUCCESS;
}