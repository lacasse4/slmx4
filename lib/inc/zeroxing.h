#ifndef _ZEROXING_H
#define _ZEROXING_H

typedef struct {
  int num_xing;
  int last_sign;
  int n_samples;
} zeroxing_t;

void  zeroxing_init(zeroxing_t* z);
int   zeroxing_put(zeroxing_t* z, float input);
float zeroxing_get(zeroxing_t* z, float sampling_rate);

#endif