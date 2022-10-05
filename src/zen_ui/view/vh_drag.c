#ifndef vh_drag_h
#define vh_drag_h

#include "view.c"
#include "wm_event.c"
#include "zc_callback.c"

typedef struct _vh_drag_t
{
    cb_t*   movecb;
    cb_t*   dropcb;
    view_t* dragged_view;
} vh_drag_t;

void vh_drag_attach(view_t* view, cb_t* movecb, cb_t* dropcb);
void vh_drag_drag(view_t* view, view_t* item);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_log.c"

void vh_drag_evt(view_t* view, ev_t ev)
{
    if (ev.type == EV_MMOVE && ev.drag)
    {
	vh_drag_t* vh = view->handler_data;

	if (vh->dragged_view)
	{
	    r2_t frame = vh->dragged_view->frame.local;
	    frame.x    = ev.x - frame.w / 2;
	    frame.y    = ev.y - frame.h / 2;
	    view_set_frame(vh->dragged_view, frame);

	    if (vh->movecb) (*vh->movecb->fp)(view, vh->dragged_view);
	}
    }
    if (ev.type == EV_MUP && ev.drag)
    {
	vh_drag_t* vh = view->handler_data;

	if (vh->dragged_view)
	{
	    if (vh->dropcb) (*vh->dropcb->fp)(view, vh->dragged_view);

	    REL(vh->dragged_view);
	    vh->dragged_view = NULL;
	}
    }
}

void vh_drag_del(void* p)
{
    vh_drag_t* vh = p;

    if (vh->dragged_view) REL(vh->dragged_view);
    if (vh->movecb) REL(vh->movecb);
    if (vh->dropcb) REL(vh->dropcb);
}

void vh_drag_desc(void* p, int level)
{
}

void vh_drag_attach(view_t* view, cb_t* movecb, cb_t* dropcb)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_drag_t* vh = CAL(sizeof(vh_drag_t), vh_drag_del, vh_drag_desc);

    if (movecb) vh->movecb = RET(movecb);
    if (dropcb) vh->dropcb = RET(dropcb);

    view->needs_touch  = 1;
    view->handler_data = vh;
    view->handler      = vh_drag_evt;
}

void vh_drag_drag(view_t* view, view_t* item)
{
    vh_drag_t* vh = view->handler_data;

    if (vh->dragged_view)
    {
	REL(vh->dragged_view);
	vh->dragged_view = NULL;
    }
    if (item)
    {
	vh->dragged_view = RET(item);
    }
}

#endif
