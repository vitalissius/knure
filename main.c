#include "message.h"
#include "util.h"

#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <bits/string_fortified.h>

int main(int argc, char* argv[])
{
    int thrs = 3;
    int msgs = 5;

    if (parse_arguments(argc, argv, &thrs, &msgs) == -1) {
        return EXIT_FAILURE;
    }

    int queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
    if (queue_id == -1) {
        return EXIT_FAILURE;
    }


    static message_t m;
    m.message_type = MESSAGE_TYPE;
    m.group_numbers = 2;
    m.character_numbers = 3;
    m.character = '4';
    m.delay = 5;
    msgsnd(queue_id, &m, MESSAGE_SIZE, IPC_NOWAIT);


    message_t m2;
    msgrcv(queue_id, &m2, MESSAGE_SIZE, MESSAGE_TYPE, IPC_NOWAIT);
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





