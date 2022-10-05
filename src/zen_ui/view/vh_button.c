#ifndef vh_button_h
#define vh_button_h

#include "view.c"
#include "zc_callback.c"

typedef enum _vh_button_type_t
{
    VH_BUTTON_NORMAL,
    VH_BUTTON_TOGGLE
} vh_button_type_t;

typedef enum _vh_button_state_t
{
    VH_BUTTON_UP,
    VH_BUTTON_DOWN
} vh_button_state_t;

typedef struct _vh_button_t
{
    cb_t*             event;
    vh_button_type_t  type;
    vh_button_state_t state;

    char    inited;
    view_t* offview;
    view_t* onview;

} vh_button_t;

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event);
void vh_button_set_state(view_t* view, vh_button_state_t state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "vh_anim.c"
#include "zc_log.c"

void vh_button_anim_end(view_t* view, void* userdata)
{
    view_t*      btnview = (view_t*) userdata;
    vh_button_t* vh      = btnview->handler_data;

    // if offview alpha is 0 and state is released

    if (vh->type == VH_BUTTON_NORMAL)
    {
	if (vh->offview->texture.alpha < 0.0001 && view == vh->offview)
	{
	    vh_anim_alpha(vh->offview, 0.0, 1.0, 10, AT_LINEAR);
	}
    }
}

void vh_button_evt(view_t* view, ev_t ev)
{
    if (ev.type == EV_TIME)
    {
	vh_button_t* vh = view->handler_data;
	if (!vh->inited)
	{
	    vh->inited = 1;
	    if (view->views->length > 0)
	    {
		vh->offview = view->views->data[0];
		vh_anim_add(vh->offview);
		vh_anim_set_event(vh->offview, view, vh_button_anim_end);
	    }
	    if (view->views->length > 1)
	    {
		vh->onview = view->views->data[1];
		vh_anim_add(vh->onview);
		vh_anim_set_event(vh->onview, view, vh_button_anim_end);
	    }

	    if (vh->offview) view_set_texture_alpha(vh->offview, 1.0, 0);
	    if (vh->onview) view_set_texture_alpha(vh->onview, 0.0, 0);
	}
    }
    else if (ev.type == EV_MDOWN)
    {
	vh_button_t* vh = view->handler_data;

	if (vh->type == VH_BUTTON_NORMAL)
	{
	    vh->state = VH_BUTTON_DOWN;

	    if (vh->offview) vh_anim_alpha(vh->offview, 1.0, 0.0, 10, AT_LINEAR);
	    if (vh->onview) vh_anim_alpha(vh->onview, 0.0, 1.0, 10, AT_LINEAR);
	}
    }
    else if (ev.type == EV_MUP)
    {
	vh_button_t* vh = view->handler_data;

	if (vh->type == VH_BUTTON_TOGGLE)
	{
	    if (vh->state == VH_BUTTON_UP)
	    {
		vh->state = VH_BUTTON_DOWN;
		if (vh->offview) vh_anim_alpha(vh->offview, 1.0, 0.0, 10, AT_LINEAR);
		if (vh->onview) vh_anim_alpha(vh->onview, 0.0, 1.0, 10, AT_LINEAR);
	    }
	    else
	    {
		vh->state = VH_BUTTON_UP;
		if (vh->offview) vh_anim_alpha(vh->offview, 0.0, 1.0, 10, AT_LINEAR);
		if (vh->onview) vh_anim_alpha(vh->onview, 1.0, 0.0, 10, AT_LINEAR);
	    }

	    if (vh->event) (*vh->event->fp)(vh->event->userdata, view);
	}
	else
	{
	    vh->state = VH_BUTTON_UP;

	    if (vh->event) (*vh->event->fp)(vh->event->userdata, view);
	    /* if (vh->offview) vh_anim_alpha(vh->offview, 1.0, 0.0, 10, AT_LINEAR); */
	    /* if (vh->onview) vh_anim_alpha(vh->onview, 0.0, 1.0, 10, AT_LINEAR); */
	}
    }
}

void vh_button_set_state(view_t* view, vh_button_state_t state)
{
    vh_button_t* vh = view->handler_data;
    vh->state       = state;

    if (state)
    {
	if (vh->offview) vh_anim_alpha(vh->offview, 1.0, 0.0, 10, AT_LINEAR);
	if (vh->onview) vh_anim_alpha(vh->onview, 0.0, 1.0, 10, AT_LINEAR);
    }
    else
    {
	if (vh->offview) vh_anim_alpha(vh->offview, 0.0, 1.0, 10, AT_LINEAR);
	if (vh->onview) vh_anim_alpha(vh->onview, 1.0, 0.0, 10, AT_LINEAR);
    }
}

void vh_button_del(void* p)
{
    vh_button_t* vh = p;
    if (vh->event) REL(vh->event);
}

void vh_button_desc(void* p, int level)
{
    printf("vh_button");
}

void vh_button_add(view_t* view, vh_button_type_t type, cb_t* event)
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_button_t* vh = CAL(sizeof(vh_button_t), vh_button_del, vh_button_desc);
    vh->event       = event;
    vh->type        = type;
    vh->state       = VH_BUTTON_UP;

    if (event) RET(event);

    view->handler      = vh_button_evt;
    view->handler_data = vh;
    view->needs_touch  = 1;
    view->blocks_touch = 1;
}

#endif
