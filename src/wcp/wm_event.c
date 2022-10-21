#ifndef event_h
#define event_h

#include <stdint.h>

enum evtype
{
    EV_EMPTY,
    EV_TIME,
    EV_RESIZE,
    EV_MMOVE,
    EV_MDOWN,
    EV_MUP,
    EV_MMOVE_OUT,
    EV_MDOWN_OUT,
    EV_MUP_OUT,
    EV_SCROLL,
    EV_KDOWN,
    EV_KUP,
    EV_TEXT,
    EV_WINDOW_SHOW,
};

typedef struct _ev_t
{
    enum evtype type;

    int x;
    int y;
    int w;
    int h;

    float dx;
    float dy;

    uint32_t time;
    uint32_t dtime;

    char text[32];
    int  drag;

    int button;
    int dclick;
    int ctrl_down;
    int shift_down;

    int keycode;
} ev_t;

#endif

#if __INCLUDE_LEVEL__ == 0

#endif
