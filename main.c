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

sem_t sem_exit;

void* producer(void* arg) {
    FILE* input_file = (FILE*) arg;

    if ((queue_id = open_message_queue(IPC_ALWAYS_NEY_QUEUE)) == -1) {
        perror("");
        pthread_exit(NULL);
    }

    while (!feof(input_file)) {
        fscanf(input_file, "%d\t%d\t%c\t%d\n", &msg.group_numbers, &msg.character_numbers, &msg.character, &msg.delay);
        msg.message_type = MESSAGE_TYPE;
        print_message(&msg);
        int res;
        if ((res = msgsnd(queue_id, &msg, MESSAGE_DATA_SIZE, 0)) == -1) {
            perror("");
        }
    }
    printf("Post...\n");
        sem_post(&sem_exit);
//    return NULL;
    pthread_exit(NULL);
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

    sem_init(&sem_exit, 0, 0);

    pthread_t thread_producer;
    pthread_create(&thread_producer, NULL, producer, input_file);
    pthread_detach(thread_producer);
//    pthread_join(thread_producer, NULL);
    printf("Wait...\n");
    sem_wait(&sem_exit);
    milli_sleep(1);
    printf("After wait...\n");
    
    sem_destroy(&sem_exit);
    
    fclose(input_file);


    static message_t m2;
    msgrcv(queue_id, &m2, MESSAGE_DATA_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
    print_message(&m2);
    msgrcv(queue_id, &m2, MESSAGE_DATA_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
    print_message(&m2);
    msgrcv(queue_id, &m2, MESSAGE_DATA_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
    print_message(&m2);
    msgrcv(queue_id, &m2, MESSAGE_DATA_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
    print_message(&m2);
    msgrcv(queue_id, &m2, MESSAGE_DATA_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
    print_message(&m2);


    int res = msgctl(queue_id, IPC_RMID, NULL);
    if (res != 0) {
        printf("Cannot remove queue\n");
    }

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





