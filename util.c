#include "util.h"

int parse_arguments(int argc, char* argv[], int* t, int* q) {
    int opt;
    while ((opt = getopt(argc, argv, "q:t:")) != -1) {
        switch (opt) {
            case 'q':
                *q = atoi(optarg);
                break;
            case 't':
                *t = atoi(optarg);
                break;
            case '?':
            default:
                fprintf(stderr, "Usage: %s [-q queue-size] [-t threads-number]\n", argv[0]);
                return -1;
        }
    }
    if (*t < 1 || *q < 1) {
        fprintf(stderr, "Queue-size and threads-number must be bigger or equal 1\n");
        return -1;
    }
    return 0;
}

int open_message_queue(key_t key, int max_size) {
    int permision_rights = 0660;
    int qid = msgget(key, IPC_CREAT | permision_rights);
    if (qid == -1) {
        return -1;
    }
    struct msqid_ds qprop;
    msgctl(qid, IPC_STAT, &qprop);
    qprop.msg_qbytes = max_size * MESSAGE_DATA_SIZE;
    msgctl(qid, IPC_SET, &qprop);
    return qid;
}

int micro_sleep(int microseconds) {
    return usleep(microseconds);
}

int milli_sleep(int milliseconds) {
    return micro_sleep(milliseconds * 1000);
}

int check_format(FILE* input_file) {
    int lines_number = 0;
    int n1, n2, n3;
    char c;
    while (1) {
        int res = fscanf(input_file, "%d\t%d\t%c\t%d\n", &n1, &n2, &c, &n3);
        if (res == 4) {
            lines_number++;
        } else if (res == EOF) {
            break;
        } else {
            return -1;
        }
    }
    rewind(input_file);
    return lines_number;
}
