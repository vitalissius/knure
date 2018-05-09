#ifndef UTIL_H
#define UTIL_H

#include <time.h>

int parse_arguments(int argc, char* argv[], int* thrs, int* msgs);

int milli_sleep(int milliseconds);

#endif /* UTIL_H */

