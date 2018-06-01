#ifndef PROCEDURES_H
#define PROCEDURES_H

#include "message.h"
#include "util.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <memory.h>

extern int g_counter;
extern int g_queue_id;
extern sem_t g_sem_exit;
extern sem_t g_sem_thrs;
extern pthread_mutex_t g_cons_mtx;
extern pthread_mutex_t g_decr_mtx;

void* producer(void* arg);

void* handler(void* arg);

void* consumer(void* arg);

#endif /* PROCEDURES_H */
