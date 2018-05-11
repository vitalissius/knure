#include "message.h"
#include "util.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int g_queue_id = 0;

/* Indicates that all messages were processed and the program may stop executing */
sem_t g_sem_exit;

sem_t g_sem_thrs;

pthread_mutex_t g_cons_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_decr_mtx = PTHREAD_MUTEX_INITIALIZER;

int g_counter = 0;

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

    pthread_mutex_lock(&g_cons_mtx);
    if (--g_counter == 0) {
        sem_post(&g_sem_exit);
    }
    pthread_mutex_unlock(&g_cons_mtx);

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

int main(int argc, char* argv[])
{
    const char* const file_name = "tasks_v3.csv";
    int handlers_max_number = 3;    // by default
    int queue_max_size = 5;         // by default

    if (parse_arguments(argc, argv, &handlers_max_number, &queue_max_size) == -1) {
        return EXIT_FAILURE;
    }

    FILE* input_stream = fopen(file_name, "r");
    if (!input_stream) {
        fprintf(stderr, "The file %s is not found!\n", file_name);
        return EXIT_FAILURE;
    }

    int lines_number;
    if ((lines_number = check_format(input_stream)) == -1) {
        fprintf(stderr, "The file's format is corrupted!\n");
        return EXIT_FAILURE;
    }
    g_counter = lines_number;

    if ((g_queue_id = open_message_queue(IPC_NEW_QUEUE, queue_max_size)) == -1) {
        perror("Something went wrong: ");
        return EXIT_FAILURE;
    }

    if (sem_init(&g_sem_exit, 0, 0) != 0) {
        perror(">: ");
        return EXIT_FAILURE;
    }

    if (sem_init(&g_sem_thrs, 0, handlers_max_number) != 0) {
        perror(">: ");
        return EXIT_FAILURE;
    }

    pthread_t thread_producer;
    pthread_create(&thread_producer, NULL, producer, input_stream);
    pthread_detach(thread_producer);

    pthread_t thread_consumer;
    pthread_create(&thread_consumer, NULL, consumer, NULL);
    pthread_detach(thread_consumer);

    sem_wait(&g_sem_exit);  /* Waiting while all handlers are running */

    int res = msgctl(g_queue_id, IPC_RMID, NULL);
    if (res != 0) {
        printf("Cannot remove queue\n");
    }

    sem_destroy(&g_sem_exit);
    sem_destroy(&g_sem_thrs);

    pthread_mutex_destroy(&g_cons_mtx);
    pthread_mutex_destroy(&g_decr_mtx);

    fclose(input_stream);

    usleep(100);    /* Time to proper end all threads */

    return 0;
}
