#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

//formatted error output, exit with EXIT_FAILURE
#define eprintf(...) do{    \
    fprintf(stderr, "ERROR: " __VA_ARGS__); \
    exit(EXIT_FAILURE); \
}while(0) \

#endif