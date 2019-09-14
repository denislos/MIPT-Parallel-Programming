

#ifndef TOOLS_H
#define TOOLS_H

#define EXPECTED_NUM_ARGUMENTS 3
#define EXPECTED_STEP_INTERVALS 1000

#include <stdio.h>
#include <stdlib.h>

static inline void raise_error(const char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

#endif // TOOLS_H