#include "utils.h"
#include <sys/time.h>

long diff_cur_time(const struct timeval* t){
    struct timeval cur_t;
    gettimeofday(&cur_t, NULL);
    return (cur_t.tv_sec - t->tv_sec) * 1e3 + ((cur_t.tv_usec - t->tv_usec) / 1e3);
}