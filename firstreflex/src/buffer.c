#include "buffer.h"

void buffer_init(buffer_t* b) 
{
    for(int i = 0; i < BUFFER_SIZE; i++) {
        b->buffer[i] = 0.0;
    }
    b->next = 0;
    b->counter = 0;
}

void buffer_put(buffer_t* b, float input) 
{
    b->buffer[b->next] = input;
    b->next++;
    if (b->next   == BUFFER_SIZE) b->next = 0;
    if (b->counter < BUFFER_SIZE) b->counter++;
}

float* buffer_get(buffer_t* b) 
{
    return b->buffer;
}

int buffer_is_valid(buffer_t* b)
{
    return b->counter == BUFFER_SIZE;
}
