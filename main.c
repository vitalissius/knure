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

int queue_id = 0;

static message_t msg;

pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t sem_exit;

void* producer(void* arg) {
    printf("Producer start\n");
    FILE* input_file = (FILE*) arg;

    while (!feof(input_file)) {
        fscanf(input_file, "%d\t%d\t%c\t%d\n", &msg.group_numbers, &msg.character_numbers, &msg.character, &msg.delay);
        msg.message_type = MESSAGE_TYPE;
        //print_message(&msg);
        int res;
        printf("try push\n");
        if ((res = msgsnd(queue_id, &msg, MESSAGE_DATA_SIZE, 0)) == -1) {
            perror("WOW: ");
        }
        printf("push ok\n");
    }
    printf("Producer exit\n");
    return NULL;//
    pthread_exit(NULL);
}

int count = 5;

void* handler(void* arg) {
    static int cnum = 0;
    cnum++;
    printf("Handler %d start\n", cnum);
    message_t* pm = (message_t*) arg;

    for (int i = 0; i < pm->group_numbers; i++) {
        pthread_mutex_lock(&console_mutex);
        for (int j = 0; j < pm->character_numbers; j++) {
            fprintf(stderr, "%c", pm->character);
            milli_sleep(pm->delay);
        }
        fprintf(stderr, "\n");
        pthread_mutex_unlock(&console_mutex);
    }

    --count;

    free(pm);

    if (count == 0) {
        sem_post(&sem_exit);
    }
    printf("Handler %d exit\n", cnum);
    return NULL;
//    pthread_exit(NULL);
}

void* consumer(void* arg) {
    printf("Consumer start\n");
    message_t m;
    while (1) {
        int res;
        
        printf("try pop\n");
        if ((res = msgrcv(queue_id, &m, MESSAGE_DATA_SIZE, MESSAGE_TYPE, 0)) != -1) {
            printf("pop ok\n");

            message_t* pm = (message_t*) malloc(sizeof(message_t));
            pm->message_type = m.message_type;
            pm->group_numbers = m.group_numbers;
            pm->character_numbers = m.character_numbers;
            pm->character = m.character;
            pm->delay = m.delay;

            pthread_t thread_handler;
            pthread_create(&thread_handler, NULL, handler, pm);
            pthread_detach(thread_handler);
        } else if (errno == EIDRM) {
            printf("Delete queue\n");
            break;
        }
    }
    printf("Consumer exit\n");
    return NULL;
//    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    char* file_name = "tasks.csv";
    int thrs = 3;
    int msgs = 5;

    if (parse_arguments(argc, argv, &thrs, &msgs) == -1) {
        return EXIT_FAILURE;
    }

    FILE* input_file = fopen(file_name, "r");
    if (!input_file) {
        fprintf(stderr, "File %s not found!\n", file_name);
        return EXIT_FAILURE;
    }
    
    if ((queue_id = open_message_queue(IPC_ALWAYS_NEY_QUEUE)) == -1) {
        perror("");
        return EXIT_FAILURE;
    }


    sem_init(&sem_exit, 0, 0);

    pthread_t thread_producer;
    pthread_create(&thread_producer, NULL, producer, input_file);
    pthread_detach(thread_producer);
//    pthread_join(thread_producer, NULL);

    pthread_t thread_consumer;
    pthread_create(&thread_consumer, NULL, consumer, NULL);
    pthread_detach(thread_consumer);
//    pthread_join(thread_consumer, NULL);


    printf("Wait...\n");
    sem_wait(&sem_exit);
    printf("After wait...\n");
    int res = msgctl(queue_id, IPC_RMID, NULL);
    if (res != 0) {
        printf("Cannot remove queue\n");
    }
    printf("Queue is removed\n");
    sem_destroy(&sem_exit);

    pthread_mutex_destroy(&console_mutex);
    fclose(input_file);

    usleep(100);// time to correct destroy all threads
    
    


    printf("Main exit\n");

    return 0;
}





//
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/ipc.h>
//#include <sys/msg.h>
//
//    struct mymsgbuf {
//            long    mtype;          /* Message type */
//            int     request;        /* Work request number */
//            double  salary;         /* Employee's salary */
//    } ;
//    
//int open_queue( key_t keyval )
//{
//        int     qid;
//        
//        if((qid = msgget( keyval, IPC_CREAT | 0660 )) == -1)
//        {
//                return(-1);
//        }
//        
//        return(qid);
//}
//
//int send_message( int qid, struct mymsgbuf *qbuf )
//{
//        int result;
//        int length;
//
//        /* The length is essentially the size of the structure minus sizeof(mtype) */
//        length = sizeof(struct mymsgbuf) - sizeof(long);        
//
//        if((result = msgsnd( qid, qbuf, length, 0)) == -1)
//        {
//                return(-1);
//        }
//        
//        return(result);
//}
//
//int read_message( int qid, long type, struct mymsgbuf *qbuf )
//{
//        int     result, length;
//
//        /* The length is essentially the size of the structure minus sizeof(mtype) */
//        length = sizeof(struct mymsgbuf) - sizeof(long);        
//
//        if((result = msgrcv( qid, qbuf, length, type,  0)) == -1)
//        {
//                return(-1);
//        }
//        
//        return(result);
//}
//
//int peek_message( int qid, long type )
//{
//        int     result;//, length;
//
//        if((result = msgrcv( qid, NULL, 0, type,  IPC_NOWAIT)) == -1)
//        {
//                if(errno == E2BIG)
//                        return(1);
//        }
//        
//        return(0);
//}
//
//int main()
//{
//    int    qid;
//    key_t  msgkey;
//
//
//    /* Generate our IPC key value */
//    msgkey = IPC_PRIVATE;//ftok(".", 'm');
//
//    /* Open/create the queue */
//    if(( qid = open_queue( msgkey)) == -1) {
//            perror("open_queue");
//            exit(1);
//    }
//
//    /* Load up the message with arbitrary test data */
////    static struct mymsgbuf msg;
////    msg.mtype   = 1;        /* Message type must be a positive number! */   
////    msg.request = 1;        /* Data element #1 */
////    msg.salary  = 1000.00;  /* Data element #2 (my yearly salary!) */
//
//    /* Bombs away! */
////    if((send_message( qid, &msg )) == -1) {
////            perror("send_message");
////            exit(1);
////    }
//    
//    static struct mymsgbuf msg;
//
//    msg.mtype   = 1;        /* Message type must be a positive number! */   
//    msg.request = 1;        /* Data element #1 */
//    msg.salary  = 1000.00;  /* Data element #2 (my yearly salary!) */
//    send_message( qid, &msg );
//    
//    msg.mtype = 1;
//    msg.request = 2;        /* Data element #1 */
//    msg.salary  = 2000.00;  /* Data element #2 (my yearly salary!) */
//    send_message( qid, &msg );
//
//    msg.mtype = 1;
//    msg.request = 3;        /* Data element #1 */
//    msg.salary  = 3000.00;  /* Data element #2 (my yearly salary!) */
//    send_message( qid, &msg );
//
//
//    struct mymsgbuf m;
//
//    read_message(qid, 1, &m);
//    printf("%ld %d %f\n", m.mtype, m.request, m.salary);
//    
//    read_message(qid, 1, &m);
//    printf("%ld %d %f\n", m.mtype, m.request, m.salary);
//
//    read_message(qid, 1, &m);
//    printf("%ld %d %f\n", m.mtype, m.request, m.salary);
//    
//    
//    //msgctl( qid, IPC_RMID, 0);
//    
//    return 0;
//}





