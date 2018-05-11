#ifndef UTIL_H
#define UTIL_H

#include "message.h"

#include <sys/msg.h>

#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define IPC_NEW_QUEUE IPC_PRIVATE

int parse_arguments(int argc, char* argv[], int* thrs, int* msgs);

int open_message_queue(key_t key, int max_size);

int micro_sleep(int microseconds);

int milli_sleep(int milliseconds);

int check_format(FILE* input_file);

#endif /* UTIL_H */

