#include <math.h>
#include "zeroxing.h"

#define SIGN_UNDEFINED -1

void zeroxing_init(zeroxing_t* z) 
{
  z->num_xing = 0;
  z->n_samples = 0;
  z->current_sign = SIGN_UNDEFINED;
}

void zeroxing_reset(zeroxing_t* z)
{
  z->num_xing = 0;
  z->n_samples = 0;
}

int zeroxing_put(zeroxing_t* z, float input) 
{
  if (z->current_sign == SIGN_UNDEFINED) {
    z->current_sign = signbit(input);
    return 0;
  }

  if (signbit(input) != z->current_sign) 
    z->num_xing++;

  return z->num_xing >= 2;
}

float zeroxing_get(zeroxing_t* z, float sampling_rate) 
{
  return z->num_xing * sampling_rate / (z->n_samples * 2);
}
