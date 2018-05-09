#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>

extern const long MESSAGE_TYPE;

/**
 * The size of useful information of message_t type (without message_type field)
 */
extern const int MESSAGE_SIZE;

typedef struct {
    long message_type;
    int  group_numbers;
    int  character_numbers;
    char character;
    int  delay;
} message_t;

int print_message(const message_t* const m);

#endif /* MESSAGE_H */

