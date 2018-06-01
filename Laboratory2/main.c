/*
 * +---------------------------------------------------------------------------+
 * | Лабораторная работа №2                                                    |
 * +---------------------------------------------------------------------------+
 * | Тема: Паралельный сервер. Потоки и их синхронизация                       |
 * | ЯП:   С                                                                   |
 * | ОС:   Linux                                                               |
 * +---------------------------------------------------------------------------+
 * | Разработать многопоточное приложение, использующее очереди сообщений и    |
 * | средства синхронизации потоков.                                           |
 * +---------------------------------------------------------------------------+
 * | Формат вызова:                                                            |
 * | $ gen_prog [options]                                                      |
 * | Необязательные опции программы:                                           |
 * | -q 3 - размер очереди сообщений;                                          |
 * | -t 2 - максимальное число потоков обработчиков.                           |
 * +---------------------------------------------------------------------------+
 * | Описание программы:                                                       |
 * |                                  ***                                      |
 * | - Поток A (поставщик)  считывает текстовый  файл  с заданиями,  формирует |
 * |   запросы и помещает их на обработку в очередь сообщений;                 |
 * |                                  ***                                      |
 * | - Число  элементов  очереди  сообщений  задаётся  в виде  необязательного |
 * |   параметра при запуске программы (по умолчанию 5);                       |
 * |                                  ***                                      |
 * | - Поток B (потребитель) читает из очереди сообщений и запускает отдельные |
 * |   потоки-обработчики для выполнения задания;                              |
 * |                                  ***                                      |
 * | - Задания  потокам-обработчикам  передаются   через  одну  или  несколько |
 * |   структур данных в памяти;                                               |
 * |                                  ***                                      |
 * | - Максимальное   число   одновременно   работающих   потоков-обработчиков |
 * |   задаётся в виде  необязательного  параметра  при  запуске программы (по |
 * |   умолчанию 3);                                                           |
 * |                                  ***                                      |
 * | - Поток-обработчик выводит информацию в поток stderr посимвольно.         |
 * |                                  ***                                      |
 * +---------------------------------------------------------------------------+
 * | Формат текстового файла с заданиями.                                      |
 * | Файл состоит из строк следующей структуры (tab-separated CSV file):       |
 * | число_групп    число_символов_в_группе    символ    задержка              |
 * | где:                                                                      |
 * | - разделителем полей служит символ табуляции;                             |
 * | - символ - символ для вывода;                                             |
 * | - числов_символов_в_группе - число подряд выводимых символов;             |
 * | - задержка  -  время  ожидания  в  миллисекундах  перед  выводом  каждого |
 * |   следующего символа группы символов;                                     |
 * | - число_групп - число групп выводимых символов.                           |
 * +---------------------------------------------------------------------------+
 */

#include "message.h"
#include "procedures.h"
#include "util.h"

#include <sys/msg.h>

int g_queue_id = 0;
int g_counter = 0;
sem_t g_sem_exit;/* Indicates that all messages were processed and the program may stop executing */
sem_t g_sem_thrs;
pthread_mutex_t g_cons_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_decr_mtx = PTHREAD_MUTEX_INITIALIZER;

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

    if ((msgctl(g_queue_id, IPC_RMID, NULL)) != 0) {
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
