#ifndef vh_tbl_evnt_h
#define vh_tbl_evnt_h

#include "vh_tbl_body.c"
#include "vh_tbl_head.c"
#include "vh_tbl_scrl.c"
#include "view.c"
#include <limits.h>

typedef enum _vh_tbl_evnt_event_t
{
    VH_TBL_EVENT_SELECT,
    VH_TBL_EVENT_OPEN,
    VH_TBL_EVENT_DRAG,
    VH_TBL_EVENT_DROP,
    VH_TBL_EVENT_KEY
} vh_tbl_evnt_event_t;

typedef struct _vh_tbl_evnt_t
{
    view_t* tbody_view;
    view_t* tscrl_view;
    view_t* thead_view;
    void*   userdata;
    int     scroll_drag;
    int     scroll_visible;
    view_t* selected_item;
    int     selected_index;
    float   sx;
    float   sy;
    void (*on_event)(view_t* view, view_t* rowview, vh_tbl_evnt_event_t event, int index, void* userdata, ev_t ev);
} vh_tbl_evnt_t;

void vh_tbl_evnt_attach(
    view_t* view,
    view_t* tbody_view,
    view_t* tscrl_view,
    view_t* thead_view,
    void (*on_event)(view_t* view, view_t* rowview, vh_tbl_evnt_event_t event, int index, void* userdata, ev_t ev),
    void* userdata);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_log.c"

#define SCROLLBAR 20.0

void vh_tbl_evnt_evt(view_t* view, ev_t ev)
{
    vh_tbl_evnt_t* vh = view->handler_data;

    if (ev.type == EV_TIME)
    {
	vh_tbl_body_t* bvh = vh->tbody_view->handler_data;

	if (bvh->items && bvh->items->length > 0)
	{
	    if (vh->sy > 0.001 || vh->sy < -0.001 || vh->sx > 0.001 || vh->sx < -0.001)
	    {
		view_t* head = vec_head(bvh->items);
		view_t* tail = vec_tail(bvh->items);

		float top = head->frame.local.y;
		float bot = tail->frame.local.y + tail->frame.local.h;
		float hth = bot - top;
		float wth = head->frame.local.w;
		float lft = head->frame.local.x;
		float rgt = head->frame.local.x + head->frame.local.w;

		vh->sx *= 0.8;
		vh->sy *= 0.8;

		float dx = vh->sx;
		float dy = vh->sy;

		if (top > 0.001) dy -= top / 5.0; // scroll back top item
		if (bot < view->frame.local.h - 0.001)
		{
		    if (hth > view->frame.local.h)
			dy += (view->frame.local.h - bot) / 5.0; // scroll back bottom item
		    else
			dy -= top / 5.0; // scroll back top item
		}
		if (lft > 0.01) dx -= lft / 5.0;
		if (rgt < view->frame.local.w - 0.01)
		{
		    if (wth > view->frame.local.w)
			dx += (view->frame.local.w - rgt) / 5.0;
		    else
			dx -= lft / 5.0;
		}

		vh_tbl_body_move(vh->tbody_view, dx, dy);
		if (vh->thead_view) vh_tbl_head_move(vh->thead_view, dx);
		if (vh->tscrl_view && vh->scroll_visible == 1) vh_tbl_scrl_update(vh->tscrl_view);
	    }

	    if (vh->tscrl_view)
	    {
		vh_tbl_scrl_t* svh = vh->tscrl_view->handler_data;
		if (svh->state > 0) vh_tbl_scrl_update(vh->tscrl_view);
	    }
	}
    }
    else if (ev.type == EV_SCROLL)
    {
	vh->sx -= ev.dx;
	vh->sy += ev.dy;
    }
    else if (ev.type == EV_RESIZE)
    {
	vh_tbl_body_move(vh->tbody_view, 0, 0);
    }
    else if (ev.type == EV_MMOVE)
    {
	// show scroll
	if (vh->scroll_visible == 0)
	{
	    vh->scroll_visible = 1;
	    if (vh->tscrl_view) vh_tbl_scrl_show(vh->tscrl_view);
	}
	if (vh->scroll_drag)
	{
	    if (ev.x > view->frame.global.x + view->frame.global.w - SCROLLBAR)
	    {
		if (vh->tscrl_view) vh_tbl_scrl_scroll_v(vh->tscrl_view, ev.y - view->frame.global.y);
	    }
	    if (ev.y > view->frame.global.y + view->frame.global.h - SCROLLBAR)
	    {
		if (vh->tscrl_view) vh_tbl_scrl_scroll_h(vh->tscrl_view, ev.x - view->frame.global.x);
	    }
	}
	if (vh->selected_item && ev.drag)
	{
	    (*vh->on_event)(view, vh->selected_item, VH_TBL_EVENT_DRAG, 0, vh->userdata, ev);

	    vh->selected_item = NULL;
	}
    }
    else if (ev.type == EV_MMOVE_OUT)
    {
	// hide scroll
	if (vh->scroll_visible == 1)
	{
	    vh->scroll_visible = 0;
	    if (vh->tscrl_view) vh_tbl_scrl_hide(vh->tscrl_view);
	}
    }
    else if (ev.type == EV_MDOWN)
    {
	if (ev.x > view->frame.global.x + view->frame.global.w - SCROLLBAR)
	{
	    vh->scroll_drag = 1;
	    if (vh->tscrl_view) vh_tbl_scrl_scroll_v(vh->tscrl_view, ev.y - view->frame.global.y);
	}
	if (ev.y > view->frame.global.y + view->frame.global.h - SCROLLBAR)
	{
	    vh->scroll_drag = 1;
	    if (vh->tscrl_view) vh_tbl_scrl_scroll_h(vh->tscrl_view, ev.x - view->frame.global.x);
	}
	if (ev.x < view->frame.global.x + view->frame.global.w - SCROLLBAR &&
	    ev.y < view->frame.global.y + view->frame.global.h - SCROLLBAR)
	{
	    vh_tbl_body_t* bvh = vh->tbody_view->handler_data;

	    for (int index = 0; index < bvh->items->length; index++)
	    {
		view_t* item = bvh->items->data[index];
		if (ev.x > item->frame.global.x &&
		    ev.x < item->frame.global.x + item->frame.global.w &&
		    ev.y > item->frame.global.y &&
		    ev.y < item->frame.global.y + item->frame.global.h)
		{
		    vh->selected_item  = item;
		    vh->selected_index = bvh->head_index + index;

		    if (!ev.dclick)
		    {
			(*vh->on_event)(view, vh->selected_item, VH_TBL_EVENT_SELECT, bvh->head_index + index, vh->userdata, ev);
		    }
		    else
		    {
			(*vh->on_event)(view, vh->selected_item, VH_TBL_EVENT_OPEN, bvh->head_index + index, vh->userdata, ev);
		    }
		    break;
		}
	    }
	}
    }
    else if (ev.type == EV_MUP)
    {
	if (ev.drag)
	{
	    if (!vh->selected_item)
	    {
		vh_tbl_body_t* bvh = vh->tbody_view->handler_data;

		int index = 0;
		// view_t* item  = NULL;
		for (index = 0; index < bvh->items->length; index++)
		{
		    view_t* item = bvh->items->data[index];
		    if (ev.x > item->frame.global.x &&
			ev.x < item->frame.global.x + item->frame.global.w &&
			ev.y > item->frame.global.y &&
			ev.y < item->frame.global.y + item->frame.global.h)
		    {
			break;
		    }
		}

		(*vh->on_event)(view, vh->selected_item, VH_TBL_EVENT_DROP, bvh->head_index + index, vh->userdata, ev);
	    }
	}
	vh->scroll_drag = 0;
    }
    else if (ev.type == EV_KDOWN)
    {
	(*vh->on_event)(view, NULL, VH_TBL_EVENT_KEY, 0, vh->userdata, ev);
    }
}

void vh_tbl_evnt_del(void* p)
{
}

void vh_tbl_evnt_desc(void* p, int level)
{
    printf("vh_tbl_evnt");
}

void vh_tbl_evnt_attach(
    view_t* view,
    view_t* tbody_view,
    view_t* tscrl_view,
    view_t* thead_view,
    void (*on_event)(view_t* view, view_t* rowview, vh_tbl_evnt_event_t event, int index, void* userdata, ev_t ev),
    void* userdata)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_tbl_evnt_t* vh = CAL(sizeof(vh_tbl_evnt_t), vh_tbl_evnt_del, vh_tbl_evnt_desc);
    vh->on_event      = on_event;
    vh->userdata      = userdata;
    vh->tbody_view    = tbody_view;
    vh->tscrl_view    = tscrl_view;
    vh->thead_view    = thead_view;

    view->handler_data = vh;
    view->handler      = vh_tbl_evnt_evt;

    view->needs_key = 1;
}

#endif
