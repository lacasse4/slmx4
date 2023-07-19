#ifndef _RMS_H
#define _RMS_H

#define RMS_TAP_NUM 15

typedef struct {
  float history[RMS_TAP_NUM];
  unsigned int last_index;
} rms_t;

void rms_init(rms_t* r, float init_value);
void rms_put(rms_t* r, float input);
float rms_get(rms_t* r);

#endif