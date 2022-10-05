/* table body view handler */

#ifndef vh_tbl_body_h
#define vh_tbl_body_h

#include "view.c"

typedef struct _vh_tbl_body_t
{
    void*  userdata;
    vec_t* items;

    float head_xpos; // horizontal position of head

    int full;       // list is full, no more elements needed
    int head_index; // index of upper element
    int tail_index; // index of lower element

    int top_index; // index of visible top element
    int bot_index; // index of visible bottom element

    view_t* (*item_create)(view_t* tview, int index, void* userdata);
    void (*item_recycle)(view_t* tview, view_t* item, void* userdata);

} vh_tbl_body_t;

void vh_tbl_body_attach(
    view_t* view,
    view_t* (*item_create)(view_t* tview, int index, void* userdata),
    void (*item_recycle)(view_t* tview, view_t* item, void* userdata),
    void* userdata);

void vh_tbl_body_move(
    view_t* view,
    float   dx,
    float   dy);

void vh_tbl_body_reset(
    view_t* view);

void vh_tbl_body_hjump(
    view_t* view,
    float   dx);

void vh_tbl_body_vjump(
    view_t* view,
    int     topindex);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_log.c"

#define TBL_BODY_PRELOAD_DISTANCE 100.0

void vh_tbl_body_del(void* p)
{
    vh_tbl_body_t* vh = p;

    // remove items

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* item = vh->items->data[index];
	view_remove_from_parent(item);
    }

    REL(vh->items);
}

void vh_tbl_body_desc(void* p, int level)
{
    printf("vh_tbl_body");
}

void vh_tbl_body_attach(
    view_t* view,
    view_t* (*item_create)(view_t* tview, int index, void* userdata),
    void (*item_recycle)(view_t* tview, view_t* item, void* userdata),
    void* userdata)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_tbl_body_t* vh = CAL(sizeof(vh_tbl_body_t), vh_tbl_body_del, vh_tbl_body_desc);
    vh->userdata      = userdata;
    vh->items         = VNEW(); // REL 0
    vh->item_create   = item_create;
    vh->item_recycle  = item_recycle;

    view->handler_data = vh;
}

void vh_tbl_body_move(
    view_t* view,
    float   dx,
    float   dy)
{
    vh_tbl_body_t* vh = view->handler_data;

    vh->full = 0;

    // repos items

    vh->head_xpos += dx;

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* iview = vh->items->data[index];
	r2_t    frame = iview->frame.local;

	frame.x = vh->head_xpos;
	frame.y += dy;

	if (frame.w < view->frame.local.w) frame.w = view->frame.local.w;

	view_set_frame(iview, frame);
    }

    // refill items

    while (vh->full == 0)
    {
	if (vh->items->length == 0)
	{
	    view_t* item = (*vh->item_create)(
		view,
		vh->head_index,
		vh->userdata);

	    if (item)
	    {
		VADD(vh->items, item);
		view_add_subview(view, item);
		view_set_frame(item, (r2_t){vh->head_xpos, 0, item->frame.local.w, item->frame.local.h});
		REL(item);
	    }
	    else
	    {
		vh->full = 1; // no more items
	    }
	}
	else
	{
	    // load head items if possible

	    view_t* head = vec_head(vh->items);

	    if (head->frame.local.y > 0.0 - TBL_BODY_PRELOAD_DISTANCE)
	    {
		view_t* item = (*vh->item_create)(
		    view,
		    vh->head_index - 1,
		    vh->userdata);

		if (item)
		{
		    vh->full = 0; // there is probably more to come
		    vh->head_index -= 1;

		    vec_ins(vh->items, item, 0);
		    view_insert_subview(view, item, 0);

		    view_set_frame(item, (r2_t){vh->head_xpos, head->frame.local.y - item->frame.local.h, item->frame.local.w, item->frame.local.h});

		    REL(item);
		}
		else
		{
		    vh->full = 1; // no more items
		}
	    }
	    else
	    {
		vh->full = 1; // no more items
	    }

	    // load tail items if possible

	    view_t* tail = vec_tail(vh->items);

	    if (tail->frame.local.y + tail->frame.local.h < view->frame.local.h + TBL_BODY_PRELOAD_DISTANCE)
	    {
		view_t* item = (*vh->item_create)(
		    view,
		    vh->tail_index + 1,
		    vh->userdata);

		if (item)
		{
		    vh->full = 0; // there is probably more to come
		    vh->tail_index += 1;

		    VADD(vh->items, item);
		    view_add_subview(view, item);

		    view_set_frame(item, (r2_t){vh->head_xpos, tail->frame.local.y + tail->frame.local.h, item->frame.local.w, item->frame.local.h});

		    REL(item);
		}
		else
		{
		    vh->full &= 1; // don't set to full if head item is added
		}
	    }
	    else
	    {
		vh->full &= 1; // don't set to full if head item is added
	    }

	    // remove items if needed

	    if (tail->frame.local.y - (head->frame.local.y + head->frame.local.h) > view->frame.local.h + 2 * TBL_BODY_PRELOAD_DISTANCE) // don't remove if list is not full
	    {
		// remove head if needed

		if (head->frame.local.y + head->frame.local.h < 0.0 - TBL_BODY_PRELOAD_DISTANCE && vh->items->length > 1)
		{
		    vh->head_index += 1;
		    view_remove_from_parent(head);
		    if (vh->item_recycle) (*vh->item_recycle)(view, head, vh->userdata);
		    VREM(vh->items, head);
		}

		// remove tail if needed

		if (tail->frame.local.y > view->frame.local.h + TBL_BODY_PRELOAD_DISTANCE && vh->items->length > 1)
		{
		    vh->tail_index -= 1;
		    view_remove_from_parent(tail);
		    if (vh->item_recycle) (*vh->item_recycle)(view, tail, vh->userdata);
		    VREM(vh->items, tail);
		}
	    }
	}
    }

    // get top and bot indexes

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* iview = vh->items->data[index];
	r2_t    frame = iview->frame.local;

	if (frame.y < 0) vh->top_index = vh->head_index + index;
	if (frame.y < view->frame.local.h) vh->bot_index = vh->head_index + index;
    }
}

void vh_tbl_body_reset(
    view_t* view)
{
    vh_tbl_body_t* vh = view->handler_data;

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* iview = vh->items->data[index];
	if (vh->item_recycle) (*vh->item_recycle)(view, iview, vh->userdata);
	view_remove_from_parent(iview);
    }

    vec_reset(vh->items);

    vh->head_index = 0;
    vh->tail_index = 0;
    vh->top_index  = 0;
    vh->bot_index  = 0;
}

void vh_tbl_body_hjump(
    view_t* view,
    float   x)
{
    vh_tbl_body_t* vh = view->handler_data;

    vh->head_xpos = x;

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* iview = vh->items->data[index];
	r2_t    frame = iview->frame.local;
	frame.x       = vh->head_xpos;
	view_set_frame(iview, frame);
    }
}

void vh_tbl_body_vjump(
    view_t* view,
    int     topindex)
{
    vh_tbl_body_t* vh = view->handler_data;

    for (int index = 0;
	 index < vh->items->length;
	 index++)
    {
	view_t* iview = vh->items->data[index];
	if (vh->item_recycle) (*vh->item_recycle)(view, iview, vh->userdata);
	view_remove_from_parent(iview);
    }

    vec_reset(vh->items);

    vh->head_index = topindex;
    vh->tail_index = topindex;
    vh->top_index  = topindex;
    vh->bot_index  = topindex;

    vh_tbl_body_move(view, 0, 0);
}

#endif
