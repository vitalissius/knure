#ifndef UTIL_H
#define UTIL_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int parse_arguments(int argc, char* argv[], int* thrs, int* msgs);

int milli_sleep(int milliseconds);

#endif /* UTIL_H */

