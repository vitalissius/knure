#include "procedures.h"

void* producer(void* arg) {
    FILE* input_stream = (FILE*) arg;

    while (!feof(input_stream)) {
        message_t m;
        /* Zeroing mem, or else valgrind begins warning about uninitialized mem*/
        memset(&m, 0, sizeof(message_t));

        fscanf(input_stream, "%d\t%d\t%c\t%d\n",
                &m.group_numbers, &m.character_numbers, &m.character, &m.delay);
        m.message_type = MESSAGE_TYPE;

        if (msgsnd(g_queue_id, &m, MESSAGE_DATA_SIZE, 0) == -1) {
            perror("Sending error: ");
            return NULL;
        }
    }
    return NULL;
}

void* handler(void* arg) {
    message_t* pm = (message_t*) arg;

    for (int i = 0; i < pm->group_numbers; i++) {
        pthread_mutex_lock(&g_cons_mtx);
        for (int j = 0; j < pm->character_numbers; j++) {
            fprintf(stderr, "%c", pm->character);
            milli_sleep(pm->delay);
        }
        fprintf(stderr, "\n");
        pthread_mutex_unlock(&g_cons_mtx);
    }

    free(pm);
    sem_post(&g_sem_thrs);

    pthread_mutex_lock(&g_decr_mtx);
    if (--g_counter == 0) {
        sem_post(&g_sem_exit);
    }
    pthread_mutex_unlock(&g_decr_mtx);

    return NULL;
}

void* consumer(void* arg) {
    message_t m;
    while (1) {
        int res;

        sem_wait(&g_sem_thrs);
        if ((res = msgrcv(g_queue_id, &m, MESSAGE_DATA_SIZE, MESSAGE_TYPE, 0)) != -1) {
            message_t* pm = (message_t*) malloc(sizeof(message_t));
            memcpy(pm, &m, sizeof(message_t));

            pthread_t thread_handler;
            pthread_create(&thread_handler, NULL, handler, pm);
            pthread_detach(thread_handler);
        } else if (errno == EIDRM) {
            /* Queue is deleted - nothing to do - exit */
            break;
        }
    }
    return NULL;
}
