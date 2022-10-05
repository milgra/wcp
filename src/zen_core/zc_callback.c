#ifndef zc_callback_h
#define zc_callback_h

#include "zc_map.c"

typedef struct _cb_t cb_t;
struct _cb_t
{
  void (*fp)(void* userdata, void* data);
  void* userdata;
};

cb_t* cb_new(void (*fp)(void*, void*), void* userdata);

#endif

#if __INCLUDE_LEVEL__ == 0

void cb_describe(void* p, int level)
{
  printf("callback");
}

cb_t* cb_new(void (*fp)(void*, void*), void* userdata)
{
  cb_t* cb     = CAL(sizeof(cb_t), NULL, cb_describe);
  cb->fp       = fp;
  cb->userdata = userdata;

  return cb;
}

#endif
