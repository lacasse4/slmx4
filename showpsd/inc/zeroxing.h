#ifndef _ZEROXING_H
#define _ZEROXING_H

#define RMS_TAP_NUM 11

typedef struct {
  int num_xing;
  int current_sign;
  int n_samples;
} zeroxing_t;

void  zeroxing_init(zeroxing_t* z);
void  zeroxing_reset(zeroxing_t* z);
int   zeroxing_put(zeroxing_t* z, float input);
float zeroxing_get(zeroxing_t* z, float sampling_rate);

#endif