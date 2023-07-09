#ifndef BUFFER_H_
#define BUFFER_H_

/*
Implementation of a circular buffer.
*/

#define BUFFER_SIZE 512

typedef struct {
    float buffer[BUFFER_SIZE];
    int   next;
    int   counter;
} buffer_t;

void   buffer_init(buffer_t* b);
void   buffer_put(buffer_t* b, float input);
float* buffer_get(buffer_t* b);
int    buffer_is_valid(buffer_t* b);

#endif