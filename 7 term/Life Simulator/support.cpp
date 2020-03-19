#include <errno.h>
#include <climits>
#include <cstring>

#include "support.h"


long get_number_from_arguments(int argc, char** argv, unsigned int pos, int (*check_correctness)(long))
{
    const int base = 10;
    char *endptr, *str;
    long val;

    if (pos > (unsigned int)argc -1)
        raise_error("Invalid format of input");

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

    if (!check_correctness(val))
        raise_error("Invalid format of the input");

    return val;
}

