#ifndef BUFFER_H_
#define BUFFER_H_

/*
Implementation of a buffer.
*/

typedef struct buffer buffer_t;

buffer_t* buffer_init(int size);
void   buffer_reset(buffer_t* b);
void   buffer_put_sample(buffer_t* b, float input);
float* buffer_get_buffer(buffer_t* b);
int    buffer_is_valid(buffer_t* b);
int    buffer_get_size(buffer_t* b);
int    buffer_get_counter(buffer_t* b);
void   buffer_release(buffer_t* b);

#endif