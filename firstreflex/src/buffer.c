#include <stdlib.h>
#include "buffer.h"

struct buffer {
    int    size;    
    int    next;
    int    counter;
    float* buffer;
};

buffer_t* buffer_init(int size) 
{
    buffer_t* b = (buffer_t*)malloc(sizeof(buffer_t));
    if (b == NULL) return NULL;

    b->buffer = (float*)malloc(sizeof(float)*size);
    if (b->buffer == NULL) {
        free(b);
        return NULL;
    } 

    b->size = size;
    buffer_reset(b);

    return b;
}

void buffer_reset(buffer_t* b)
{
    b->next = 0;
    b->counter = 0;
    for(int i = 0; i < b->size; i++) {
        b->buffer[i] = 0.0;
    }
}

void buffer_put_sample(buffer_t* b, float input) 
{
    b->buffer[b->next] = input;
    b->next++;
    if (b->next   == b->size) b->next = 0;
    if (b->counter < b->size) b->counter++;
}

float* buffer_get_buffer(buffer_t* b) 
{
    return b->buffer;
}

int buffer_is_valid(buffer_t* b)
{
    return b->counter == b->size;
}

int buffer_get_counter(buffer_t* b)
{
    return b->counter;
}

int buffer_get_size(buffer_t* b)
{
    return b->size;
}

void buffer_release(buffer_t* b)
{
    free(b->buffer);
    free(b);
}
