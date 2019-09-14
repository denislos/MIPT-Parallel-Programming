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
#define CPU_MODE_STRING_SIZE 2


int check_passed_number_procs(long number) { return number >= 0; }
unsigned int get_cpu_mode();

enum CPU_MODES {
    INVALID_MODE = 0,
    MODE_64_BIT  = 64,
    MODE_32_BIT  = 32,
    MODE_16_BIT  = 16 // the minimum CPU mode to be supported by Linux kernel w/o very special hacks
};

int main(int argc, char** argv)
{
    if (argc != 2)
        raise_error("Invalid format");

    const int N = (int)get_number_from_arguments(argc, argv, POSITION_INPUT_NUM_THREADS, check_passed_number_procs);
    
    // get line size
    const long L1_linesize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (L1_linesize == -1)
        raise_error("Bad sysconf call");

    //get CPU mode (64-bit, 32-bit, 16-bit)
    const unsigned int cpu_mode = get_cpu_mode();

    if ((long)cpu_mode > L1_linesize * sizeof(uint8_t))
        raise_error("Interesting...Not atomic write though");

    omp_set_num_threads(N);

    uint16_t global_variable_16bit_mode = 0;
    uint32_t global_variable_32bit_mode = 0;
    uint64_t global_variable_64bit_mode = 0;


    #pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        int is_executed = 0;

        #define PARALLEL_EXECUTION_CODE_CPU_MODE(CPU_MODE)                                        \
            do {                                                                                  \
                while(!is_executed)                                                               \
                {                                                                                 \
                    if ((uint##CPU_MODE##_t)thread_id == global_variable_##CPU_MODE##bit_mode)    \
                    {                                                                             \
                        if (fprintf(stderr, "%d\n", thread_id) == -1)                               \
                            raise_error("Bad fprintf");                                           \
                        global_variable_##CPU_MODE##bit_mode++;                                   \
                        is_executed = 1;                                                          \
                    }                                                                             \
                }                                                                                 \
            } while(0)
        
        if (cpu_mode == MODE_64_BIT)
            PARALLEL_EXECUTION_CODE_CPU_MODE(64);
        else if (cpu_mode == MODE_32_BIT)
            PARALLEL_EXECUTION_CODE_CPU_MODE(32);
        else
            PARALLEL_EXECUTION_CODE_CPU_MODE(16);

        #undef PARALLEL_EXECUTION_CODE_CPU_MODE                    
    }

    return EXIT_SUCCESS;
}


unsigned int get_cpu_mode()
{
    FILE* file = popen("/bin/bash -c lscpu | grep Flags: | grep -owE \"lm|tm|rm\"", "r");
    if (file == NULL)
        raise_error("Bad popen");

    char cpu_mode[CPU_MODE_STRING_SIZE];
    int mode = INVALID_MODE;

    while (fscanf(file, "%2s", cpu_mode) != -1)
    {
        if (strncmp(cpu_mode, "lm", CPU_MODE_STRING_SIZE) == 0)
        {
            mode = MODE_64_BIT;
        }
        else if (strncmp(cpu_mode, "tm", CPU_MODE_STRING_SIZE) == 0 && mode != MODE_32_BIT)
        {
            mode = MODE_32_BIT;
        }
        else if (strncmp(cpu_mode, "rm", CPU_MODE_STRING_SIZE) == 0 && mode == INVALID_MODE)
        {
            mode = MODE_16_BIT;
        }
        else
            raise_error("Current system is not supported");
    }

    if (mode == INVALID_MODE)
        raise_error("Invalid system read");

    if (pclose(file) == -1)
        raise_error("Bad pclose");

    return mode;
}


