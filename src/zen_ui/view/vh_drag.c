#ifndef vh_drag_h
#define vh_drag_h

#include "view.c"
#include "wm_event.c"

enum vh_drag_event_id
{
    VH_DRAG_MOVE,
    VH_DRAG_DROP
};

typedef struct _vh_drag_t vh_drag_t;

typedef struct _vh_drag_event_t
{
    enum vh_drag_event_id id;
    vh_drag_t*            vh;
    view_t*               view;
    view_t*               dragged_view;
} vh_drag_event_t;

struct _vh_drag_t
{
    void (*on_event)(vh_drag_event_t);
    view_t* dragged_view;
};

void vh_drag_attach(view_t* view, void (*on_event)(vh_drag_event_t));
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

	    vh_drag_event_t event = {.id = VH_DRAG_MOVE, .vh = vh, .view = view, .dragged_view = vh->dragged_view};
	    if (vh->on_event) (*vh->on_event)(event);
	}
    }
    if (ev.type == EV_MUP && ev.drag)
    {
	vh_drag_t* vh = view->handler_data;

	if (vh->dragged_view)
	{
	    vh_drag_event_t event = {.id = VH_DRAG_DROP, .vh = vh, .view = view, .dragged_view = vh->dragged_view};
	    if (vh->on_event) (*vh->on_event)(event);

	    REL(vh->dragged_view);
	    vh->dragged_view = NULL;
	}
    }
}

void vh_drag_del(void* p)
{
    vh_drag_t* vh = p;

    if (vh->dragged_view) REL(vh->dragged_view);
}

void vh_drag_desc(void* p, int level)
{
}

void vh_drag_attach(view_t* view, void (*on_event)(vh_drag_event_t))
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_drag_t* vh = CAL(sizeof(vh_drag_t), vh_drag_del, vh_drag_desc);
    vh->on_event  = on_event;

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
