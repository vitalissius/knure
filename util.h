#ifndef UTIL_H
#define UTIL_H

#include "message.h"

#include <sys/ipc.h>
#include <sys/msg.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define IPC_ALWAYS_NEY_QUEUE IPC_PRIVATE

int parse_arguments(int argc, char* argv[], int* thrs, int* msgs);

int open_message_queue(key_t key);

int milli_sleep(int milliseconds);

#endif /* UTIL_H */

