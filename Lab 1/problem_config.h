

#ifndef PROBLEM_CONFIG_H
#define PROBLEM_CONFIG_H

#include <math.h>

static inline double f(double x, double t)
{
    return sin(x * x) + cos(t) + x * t * t;
}

static inline double phi(double x)
{
    return sin(x) + cos(x * x + x);
}

static inline double psi(double t)
{
    return t * t + t + 1;
}


#define X_MAX 1
#define T_MAX 1


#endif // PROBLEM_CONFIG_H