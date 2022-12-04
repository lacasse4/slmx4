#ifndef _RMS_H
#define _RMS_H

#define RMS_TAP_NUM 11

typedef struct {
  float history[RMS_TAP_NUM];
  unsigned int last_index;
} rms_t;

void rms_init(rms_t* r);
void rms_put(rms_t* r, float input);
float rms_get(rms_t* r);

#endif