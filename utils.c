#include "utils.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void print_err(const char* fmt, ...){
    va_list va;
    va_start(va, fmt);
    
    vfprintf(stderr,fmt, va);
    va_end(va);
    exit(EXIT_FAILURE);
}