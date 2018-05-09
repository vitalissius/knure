#include "util.h"

int milli_sleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000 * 1000;
    return nanosleep(&ts, NULL);
}
