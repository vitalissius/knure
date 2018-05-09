#include "message.h"
#include "util.h"

#include <sys/msg.h>
#include <sys/time.h>

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int thrs = 3;
    int msgs = 5;

    if (parse_arguments(argc, argv, &thrs, &msgs) == -1) {
        return EXIT_FAILURE;
    }


    

    return 0;
}
