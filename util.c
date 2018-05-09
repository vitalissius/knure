#include "util.h"

int parse_arguments(int argc, char* argv[], int* thrs, int* msgs) {
    int opt;
    while ((opt = getopt(argc, argv, "q:t:")) != -1) {
        switch (opt) {
            case 'q':
                *msgs = atoi(optarg);
                break;
            case 't':
                *thrs = atoi(optarg);
                break;
            case '?':
            default:
                fprintf(stderr, "Usage: %s [-q messages-number] [-t threads-number]\n", argv[0]);
                return -1;
        }
    }
    return 0;
}

int milli_sleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
    return nanosleep(&ts, NULL);
}
