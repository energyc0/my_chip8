#ifndef UTILS_H
#define UTILS_H

#include "display.h"
#include <stdio.h>
#include <stdlib.h>

//formatted error output, exit with EXIT_FAILURE
#define eprintf(...) do{    \
    fprintf(stderr, "ERROR: " __VA_ARGS__); \
    exit(EXIT_FAILURE); \
}while(0) \

//return time difference in ms between t and current time
long diff_cur_time(const struct timeval* t);

#endif