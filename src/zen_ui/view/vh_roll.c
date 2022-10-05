#ifndef vh_roll_h
#define vh_roll_h

#include "view.c"
#include "wm_event.c"
#include "zc_callback.c"

typedef struct _vh_roll_t
{
  char  active;
  cb_t* roll_in;
  cb_t* roll_out;
} vh_roll_t;

void vh_roll_add(view_t* view, cb_t* roll_in, cb_t* roll_out);

#endif

#if __INCLUDE_LEVEL__ == 0

void vh_roll_evt(view_t* view, ev_t ev)
{
  if (ev.type == EV_MMOVE)
  {
    vh_roll_t* vh    = view->handler_data;
    r2_t       frame = view->frame.global;

    if (!vh->active)
    {
      if (ev.x >= frame.x &&
          ev.x <= frame.x + frame.w &&
          ev.y >= frame.y &&
          ev.y <= frame.y + frame.h)
      {
        vh->active = 1;
        (*vh->roll_in->fp)(vh->roll_in->userdata, view);
      }
    }
  }
  else if (ev.type == EV_MMOVE_OUT)
  {
    vh_roll_t* vh    = view->handler_data;
    r2_t       frame = view->frame.global;

    if (vh->active)
    {
      if (ev.x < frame.x ||
          ev.x > frame.x + frame.w ||
          ev.y < frame.y ||
          ev.y > frame.y + frame.h)
      {
        vh->active = 0;
        (*vh->roll_out->fp)(vh->roll_out->userdata, view);
      }
    }
  }
}

void vh_roll_del(void* p)
{
  vh_roll_t* vh = p;
  if (vh->roll_in) REL(vh->roll_in);
  if (vh->roll_out) REL(vh->roll_out);
}

void vh_roll_desc(void* p, int level)
{
  printf("vh_roll");
}

void vh_roll_add(view_t* view, cb_t* roll_in, cb_t* roll_out)
{
  assert(view->handler == NULL && view->handler_data == NULL);

  vh_roll_t* vh = CAL(sizeof(vh_roll_t), vh_roll_del, vh_roll_desc);
  vh->roll_in   = roll_in;
  vh->roll_out  = roll_out;

  if (roll_in) RET(roll_in);
  if (roll_out) RET(roll_out);

  view->handler_data = vh;
  view->handler      = vh_roll_evt;
}

#endif
