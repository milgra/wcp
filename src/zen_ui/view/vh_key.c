#ifndef vh_key_h
#define vh_key_h

#include "view.c"
#include "zc_callback.c"

void vh_key_add(view_t* view, cb_t* on_key);

#endif

#if __INCLUDE_LEVEL__ == 0

typedef struct _vh_key_t
{
  cb_t* on_key;
} vh_key_t;

void vh_key_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_KUP)
  {
    vh_key_t* vh = view->handler_data;
    (*vh->on_key->fp)(vh->on_key->userdata, &ev);
  }
}

void vh_key_del(void* p)
{
  vh_key_t* vh = p;
  if (vh->on_key) REL(vh->on_key);
}

void vh_key_desc(void* p, int level)
{
  printf("vh_key");
}

void vh_key_add(view_t* view, cb_t* on_key)
{
  assert(view->handler == NULL && view->handler_data == NULL);

  vh_key_t* vh = CAL(sizeof(vh_key_t), vh_key_del, vh_key_desc);
  vh->on_key   = on_key;

  if (on_key) RET(on_key);

  view->needs_key    = 1;
  view->handler_data = vh;
  view->handler      = vh_key_evt;
}

#endif
