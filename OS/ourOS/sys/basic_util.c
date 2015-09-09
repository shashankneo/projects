#include <sys/basic_util.h>

uint64_t positive_diff(int64_t a, int64_t b)
{
    int64_t diff = a - b;
    return diff < 0 ? -diff : diff;
}