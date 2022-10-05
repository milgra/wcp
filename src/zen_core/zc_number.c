#ifndef zc_number_h
#define zc_number_h

#include <stdint.h>

typedef union
{
  float    floatv;
  int      intv;
  uint32_t uint32v;
} num_t;

num_t* num_new_float(float val);
num_t* num_new_int(int val);
num_t* num_new_uint32(uint32_t val);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_memory.c"

void num_describe(void* p, int level)
{
  num_t* num = p;
  printf("num %f %i %u", num->floatv, num->intv, num->uint32v);
}

num_t* num_new_float(float val)
{
  num_t* res  = CAL(sizeof(num_t), NULL, num_describe);
  res->floatv = val;
  return res;
}

num_t* num_new_int(int val)
{
  num_t* res = CAL(sizeof(num_t), NULL, num_describe);
  res->intv  = val;
  return res;
}

num_t* num_new_uint32(uint32_t val)
{
  num_t* res   = CAL(sizeof(num_t), NULL, num_describe);
  res->uint32v = val;
  return res;
}

#endif
