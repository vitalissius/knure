#include "message.h"

const long MESSAGE_TYPE = 0xC0FFEE;

const int MESSAGE_DATA_SIZE = sizeof(message_t) - sizeof(long);

int print_message(const message_t* const m) {
    return printf("%#lx\t%d\t%d\t%c\t%d\n",
            m->message_type,
            m->group_numbers, m->character_numbers, m->character, m->delay);
}
