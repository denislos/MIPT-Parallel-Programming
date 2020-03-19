
#ifndef PLAYGRND_SUPPORT_H
#define PLAYGRND_SUPPORT_H

#include <cstdio>
#include <cstdlib>

static inline void raise_error(const char* error_message)
{
    fprintf(stderr, "ERROR: %s\n", error_message);
    exit(EXIT_FAILURE);
}

long get_number_from_arguments(int argc, char** argv, unsigned int pos, int (*check_correctness)(long));


#endif // PLAYGRND_SUPPORT_H