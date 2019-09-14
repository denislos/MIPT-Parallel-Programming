
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <pthread.h>

#include "tools.h"

double get_precision_from_arguments(int argc, char** argv);
int get_num_procs_from_arguments(int argc, char** argv);

long get_integer_number_from_arguments(char** argv, unsigned int pos);
double get_double_number_from_arguments(char** argv, unsigned int pos);

#define START_POINT 0.001
#define END_POINT   1.0
#define NUM_TREND_INTERVALS 319

static inline double f(double x)
{
    return sin(1 / x);
}


// global integral sum
double global_sum = 0;
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct thread_data_t
{
    double standard_granulation_step;
    size_t standard_granulation_start;
    size_t standard_granulation_num_steps;

    size_t dynamic_granulation_start;
    size_t dynamic_granulation_num_steps;

    double dynamic_granulation_start_point;
    double dynamic_granulation_end_point;

    int use_last_standard_step;

} thread_data_t;


void* calculate(void* args)
{
    const thread_data_t data = *(thread_data_t*)args;

    const double standard_granulation_step = data.standard_granulation_step;
    const size_t standard_granulation_start = data.standard_granulation_start;
    const size_t standard_granulation_num_steps = data.standard_granulation_num_steps;

    const size_t dynamic_granulation_start = data.dynamic_granulation_start;
    const size_t dynamic_granulation_num_steps = data.dynamic_granulation_num_steps;

    const double dynamic_granulation_end_point = data.dynamic_granulation_end_point;

    const int use_last_standard_step = data.use_last_standard_step;

    double sum = 0;
    double current_point = 0;
    double current_step = 0;
    
    if (dynamic_granulation_start == 0)
    {
        current_step = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2) * M_PI) - START_POINT;
    }
    else
    {
        current_step = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2 - dynamic_granulation_start) * M_PI) - 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - dynamic_granulation_start) * M_PI);
    }


    if (dynamic_granulation_start == 0)
    {
        current_point = START_POINT;
    }
    else
    {
        current_point = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - dynamic_granulation_start) * M_PI);
    }
    

    //printf("zero %lg\n", current_point);

    size_t i = 0;
    for (i = dynamic_granulation_start; i < dynamic_granulation_num_steps + dynamic_granulation_start; i++)
    {
        sum += (f(current_point) + f(current_point + current_step)) * 0.5 * current_step;
        //printf("dynamic sum %lg\n", sum);
        current_point = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - i) * M_PI);
        //printf("dynamic current_point %lg\n", current_point);
        current_step = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2 - i) * M_PI) - 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - i) * M_PI);
    }

    const size_t standard_granulation_end = (use_last_standard_step) ? (standard_granulation_start + standard_granulation_num_steps - 1) : (standard_granulation_start + standard_granulation_num_steps);


    current_point = dynamic_granulation_end_point + (standard_granulation_start) * standard_granulation_step;
    //printf("first %lg %d\n", current_point, use_last_standard_step);

    for (i = standard_granulation_start; i < standard_granulation_end; i++)
    {
        sum += (f(current_point) + f(current_point + standard_granulation_step)) * 0.5 * standard_granulation_step;
        //printf("standard current point %lg\n", current_point);
        current_point += standard_granulation_step;
        //printf("Sum iter: %lg\n", sum);
    }

    //current_point -= standard_granulation_step;
    //printf("step: %lg\n", standard_granulation_step);

    //printf("%lg\n", current_point);

    if (use_last_standard_step)
    {
        const double last_standart_step = END_POINT - current_point;
        //printf("Before sum %lg Last step: %lg\n", sum, last_standart_step);
        sum += (f(current_point) + f(current_point + last_standart_step))*0.5*last_standart_step;
        //printf("After sum %lg\n", sum);
    }

    //printf("final sum in thread: %lg\n", sum);

    pthread_mutex_lock(&global_mutex);
    global_sum += sum;
    pthread_mutex_unlock(&global_mutex);

    return NULL;
}



int main(int argc, char** argv)
{
    const int num_procs    = get_num_procs_from_arguments(argc, argv);
    const double precision = get_precision_from_arguments(argc, argv);

    pthread_t* pthreads = (pthread_t*)calloc(sizeof(*pthreads), num_procs);
    if (pthreads == NULL)
        raise_error("Bad alloc");
    
    thread_data_t* pthread_data = (thread_data_t*)calloc(sizeof(*pthread_data), num_procs);
    if (pthread_data == NULL)
        raise_error("Bad alloc");
    
    const double expected_step = sqrt(12.0 / EXPECTED_STEP_INTERVALS * precision / (END_POINT - START_POINT));

    double current_step = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2) * M_PI) - START_POINT;

    size_t dynamic_granulation_num_steps = 0;

    double standard_dynamic_step_threshold = 2 * expected_step;

    while (current_step < standard_dynamic_step_threshold && dynamic_granulation_num_steps < NUM_TREND_INTERVALS - 1)
    {
        dynamic_granulation_num_steps++;
        current_step = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2 - dynamic_granulation_num_steps) * M_PI) - 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - dynamic_granulation_num_steps) * M_PI);
        //printf("%lg %lg\n", expected_step, current_step);
    }

    const double dynamic_granulation_end_point = 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 2 - dynamic_granulation_num_steps) * M_PI);

    //printf("Using dynamic granulation for %ld intervals\n", dynamic_granulation_num_steps);
    
    size_t standard_granulation_num_steps = 0;
    if (dynamic_granulation_num_steps != 0)
    {
        standard_granulation_num_steps = (size_t)((END_POINT - 1 / (M_PI / 2 + (NUM_TREND_INTERVALS - 1 - dynamic_granulation_num_steps) * M_PI)) / expected_step);
    }
    else
    {
        standard_granulation_num_steps = (size_t)((END_POINT - START_POINT)/ expected_step);
    }

    size_t global_standard_granulation_distrib_counter = standard_granulation_num_steps + 1;
    size_t global_dynamic_granulation_distrib_counter  = dynamic_granulation_num_steps;

    const size_t ordinary_granulation_per_thread = (global_standard_granulation_distrib_counter + global_dynamic_granulation_distrib_counter) / (size_t)num_procs;
    
    //printf("Num standard %ld Granulation per thread %ld\n", global_standard_granulation_distrib_counter,  ordinary_granulation_per_thread);

    int i = 0;

    
    for (i = 0; i < num_procs; i++)
    {
        size_t standard_distrib_counter = 0;
        size_t dynamic_distrib_counter = 0;

        size_t dynamic_start  = 0;
        size_t standard_start = 0;

        int use_last_standard_step = 0;
        
        const size_t target_distrib_counter_value = (i != num_procs - 1) ? ordinary_granulation_per_thread : (standard_granulation_num_steps + 1 + dynamic_granulation_num_steps - ordinary_granulation_per_thread * (num_procs - 1));

        // For dynamic granulation
        if (global_dynamic_granulation_distrib_counter != 0)
        {
            dynamic_start = (dynamic_granulation_num_steps - global_dynamic_granulation_distrib_counter);

            if (target_distrib_counter_value > global_dynamic_granulation_distrib_counter)
            {
                dynamic_distrib_counter += global_dynamic_granulation_distrib_counter;
                global_dynamic_granulation_distrib_counter = 0;
            }
            else
            {
                dynamic_distrib_counter += target_distrib_counter_value;
                global_dynamic_granulation_distrib_counter -= target_distrib_counter_value;
            }
        }

        // For standard granulation
        if (dynamic_distrib_counter != target_distrib_counter_value)
        {
            standard_start = (standard_granulation_num_steps + 1 - global_standard_granulation_distrib_counter);

            standard_distrib_counter += (target_distrib_counter_value - dynamic_distrib_counter);
            global_standard_granulation_distrib_counter -= standard_distrib_counter;
        }

        if (global_standard_granulation_distrib_counter == 0)
        {
            use_last_standard_step = 1; 
        }

        pthread_data[i] = (thread_data_t) {
            .standard_granulation_step = expected_step,
            .standard_granulation_start = standard_start,
            .standard_granulation_num_steps = standard_distrib_counter,

            .dynamic_granulation_start = dynamic_start,
            .dynamic_granulation_num_steps = dynamic_distrib_counter,
            
            .dynamic_granulation_end_point = dynamic_granulation_end_point,

            .use_last_standard_step = use_last_standard_step
        };

        if (pthread_create(&pthreads[i], NULL, calculate, &pthread_data[i]) == -1)
            raise_error("Bad pthread create");

        //printf("dynamic start: %ld num: %ld standard start: %ld num %ld\n", dynamic_start, dynamic_distrib_counter, standard_start, standard_distrib_counter);
    }

    for (i = 0; i < num_procs; i++)
    {
        if (pthread_join(pthreads[i], NULL) == -1)
            raise_error("Bad pthread join");
    }

    printf("Integral: %lg\n", global_sum);

    return EXIT_SUCCESS;
}

double get_precision_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGUMENTS)
        raise_error("Usage [num_procs] [precision]");

    const double val = get_double_number_from_arguments(argv, 2);

    if (val <= 0)
        raise_error("precision should be a positive number");

    return val;
}


int get_num_procs_from_arguments(int argc, char** argv)
{
    if (argc != EXPECTED_NUM_ARGUMENTS)
        raise_error("Usage [num_procs] [precision]");

    const long val = get_integer_number_from_arguments(argv, 1);
    
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


double get_double_number_from_arguments(char** argv, unsigned int pos)
{
    char *endptr, *str;
    double val;

    str = argv[pos];

    errno = 0;
    val = strtod(str, &endptr);

    if (endptr == str)
        raise_error("No digits were found in N");

    if (*endptr != '\0')
        raise_error("Further characters after N");

    return val;
}
