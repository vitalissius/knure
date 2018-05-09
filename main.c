#include "message.h"
#include "util.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef struct {
    int mq_id;              // message queue identifier
} producer_arg_t;

typedef struct {
    int mq_id;              // message queue identifier
    pthread_mutex_t* mtx;   // mutex for sync-output to console
    sem_t* exit_sem;
    sem_t* thrs_sem;
    int* mn;
} consumer_arg_t;

typedef struct {
    message_t* msg;
    pthread_mutex_t* mtx;
    sem_t* exit_sem;
    sem_t* thrs_sem;
    int* mn;
} handler_arg_t;

void* producer(void* arg)
{
    producer_arg_t* producer_arg = (producer_arg_t*) arg;

    FILE* input_file = fopen("tasks.csv", "r");
    if (!input_file) {
        printf("Bad file\n");
        pthread_exit(NULL);
    }

    while (!feof(input_file)) {
        static message_t m;
        fscanf(input_file, "%d\t%d\t%c\t%d\n", &m.group_numbers, &m.character_numbers, &m.character, &m.delay);
        m.message_type = MESSAGE_TYPE;
        print_message(&m);
        msgsnd(producer_arg->mq_id, &m, MESSAGE_SIZE, 0);
    }
    pthread_exit(NULL);
}

void* handler(void* arg)
{
    handler_arg_t* handler_arg = (handler_arg_t*) arg;
    message_t* m = handler_arg->msg;
    pthread_mutex_t* mutex = handler_arg->mtx;
    sem_t* exit_sem = handler_arg->exit_sem;
    sem_t* thrs_sem = handler_arg->thrs_sem;

    for (int i = 0; i < m->group_numbers; i++) {
        pthread_mutex_lock(mutex);
        for (int j = 0; j < m->character_numbers; j++) {
            fprintf(stderr, "%c", m->character);
            milli_sleep(m->delay);
        }
        fprintf(stderr, "\n");
        pthread_mutex_unlock(mutex);
    }
    sem_post(thrs_sem);

    if ((--(*handler_arg->mn)) == 0) {
        free(handler_arg->msg);
        free(handler_arg);
        printf(">>>Posting...\n");
        sem_post(exit_sem);
    } else {
        free(handler_arg->msg);
        free(handler_arg);
    }

    pthread_exit(NULL);
}

void* consumer(void* arg)
{
    consumer_arg_t* consumer_arg = (consumer_arg_t*) arg;
    sem_t* thrs_sem = consumer_arg->thrs_sem;

    while (1) {
        message_t m;
        //int res = 
        msgrcv(consumer_arg->mq_id, &m, MESSAGE_SIZE, MESSAGE_TYPE, 0);
        //if (res != -1) {
            sem_wait(thrs_sem);

            handler_arg_t* handler_arg = malloc(sizeof(*handler_arg));
            handler_arg->msg = malloc(sizeof(*(handler_arg->msg)));
            handler_arg->mtx = consumer_arg->mtx;
            handler_arg->exit_sem = consumer_arg->exit_sem;
            handler_arg->thrs_sem = consumer_arg->thrs_sem;
            handler_arg->mn = consumer_arg->mn;

            memcpy(handler_arg->msg, &m, sizeof(m));

            pthread_t thread_handler;   // This is OK. It's only an identifier of the thread and if in the future we do not use it in the function pthread_join(), etc., then it may be removed.
            pthread_create(&thread_handler, NULL, handler, handler_arg);
            pthread_detach(thread_handler);
            //pthread_join(thread_handler, NULL);
        //}
    }
    pthread_exit(NULL);
}

int main(int argc, char* args[])
{
    const int threads_number = 3;
    const int messages_number = 5;
    if (threads_number > messages_number) {
        perror("Number of threads must be less or equal number of messages\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
    sem_t exits_semaphore;
    sem_t threads_semaphore;
    sem_init(&exits_semaphore, 0, 0);
    sem_init(&threads_semaphore, 0, threads_number);

    //g_handled_messages = messages_number;


    int queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660);

    pthread_t thread_producer;
    producer_arg_t producer_arg;
    producer_arg.mq_id = queue_id;

    pthread_t thread_consumer;
    consumer_arg_t consumer_arg;
    consumer_arg.mq_id = queue_id;
    consumer_arg.mtx = &console_mutex;
    consumer_arg.exit_sem = &exits_semaphore;
    consumer_arg.thrs_sem = &threads_semaphore;
    int mn = messages_number;
    consumer_arg.mn = &mn;

    pthread_create(&thread_producer, NULL, producer, &producer_arg);
    //pthread_detach(thread_producer);
    pthread_join(thread_producer, 0);
    pthread_create(&thread_consumer, NULL, consumer, &consumer_arg);
    pthread_detach(thread_consumer);
//    pthread_join(thread_producer, NULL);
//    pthread_join(thread_consumer, NULL);

    printf(">>>Waiting...\n");
    sem_wait(&exits_semaphore);


    msgctl(queue_id, IPC_RMID, NULL);
    pthread_mutex_destroy(&console_mutex);
    sem_destroy(&threads_semaphore);
    sem_destroy(&exits_semaphore);
    
    

    return 0;
}
