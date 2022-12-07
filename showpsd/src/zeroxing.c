#include <math.h>
#include "zeroxing.h"

#define SIGN_UNDEFINED -1

void zeroxing_init(zeroxing_t* z) 
{
  z->num_xing = 0;
  z->n_samples = 0;
  z->last_sign = SIGN_UNDEFINED;
}

void zeroxing_reset(zeroxing_t* z)
{
  z->num_xing = 1;
  z->n_samples = 0;
}

int zeroxing_put(zeroxing_t* z, float input) 
{
  int current_sign = signbit(input);
  if (z->last_sign == SIGN_UNDEFINED) {
    z->last_sign = current_sign;
    return 0;
  }

  if (z->last_sign != current_sign) {
    z->num_xing++;
    z->last_sign = current_sign;
  }

  if (z->num_xing != 0) 
    z->n_samples++;

  return z->num_xing >= 3;
}

float zeroxing_get(zeroxing_t* z, float sampling_rate) 
{
  int n_samples = z->n_samples;
  zeroxing_reset(z);
  return sampling_rate / n_samples * 2;
}

