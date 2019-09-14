

#ifndef LISIM_TOOLS_H
#define LISIM_TOOLS_H

#include <iostream>

inline void raise_error(const char* message)
{
    std::cerr << message << std::endl;
    exit(EXIT_FAILURE);
}


#endif // LISIM_TOOLS_H