#ifndef BREATHFILTER_H_
#define BREATHFILTER_H_

/*

Small FIR low pass filter designed empiricaly.

*/

#define BREATHFILTER_TAP_NUM 5

typedef struct {
  float history[BREATHFILTER_TAP_NUM];
  unsigned int last_index;
} breathFilter;

void breathFilter_init(breathFilter* f);
void breathFilter_put(breathFilter* f, float input);
float breathFilter_get(breathFilter* f);

#endif