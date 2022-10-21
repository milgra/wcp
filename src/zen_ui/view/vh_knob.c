#ifndef vh_knob_h
#define vh_knob_h

#include "view.c"

void vh_knob_add(view_t* view, void (*ratio_changed)(view_t* view, float ratio));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_knob.c"
#include "zc_draw.c"
#include <stdio.h>

typedef struct _vh_knob_t
{
    float      angle;
    char*      id;
    bm_rgba_t* top;
    bm_rgba_t* bottom;

    void (*ratio_changed)(view_t* view, float ratio);
} vh_knob_t;

void vh_knob_evt(view_t* view, ev_t ev)
{
    if (ev.type == EV_MDOWN || (ev.type == EV_MMOVE && ev.drag))
    {
	vh_knob_t* vh = view->handler_data;

	float dx    = ev.x - (view->frame.global.x + view->frame.global.w / 2.0);
	float dy    = ev.y - (view->frame.global.y + view->frame.global.h / 2.0);
	float angle = atan2(dy, dx);
	float r     = sqrt(dx * dx + dy * dy);

	if (angle < 0) angle += 6.28;

	if (r < view->frame.global.w / 2.0)
	{
	    tg_knob_set_angle(view, angle);
	    (*vh->ratio_changed)(view, angle);
	}
    }
    else if (ev.type == EV_SCROLL)
    {
	vh_knob_t* vh = view->handler_data;
	tg_knob_t* tg = view->tex_gen_data;

	float angle = tg->angle - ev.dy / 50.0;

	if (angle < 0) angle += 6.28;

	if (tg->angle < 3 * 3.14 / 2 && tg->angle > 3.14 && angle > 3 * 3.14 / 2) angle = tg->angle;
	if (tg->angle > 3 * 3.14 / 2 && tg->angle < 2 * 3.14 && angle < 3 * 3.14 / 2) angle = tg->angle;

	if (angle > 6.28) angle -= 6.28;
	tg_knob_set_angle(view, angle);
	(*vh->ratio_changed)(view, angle);
    }
}

void vh_knob_desc(void* p, int level)
{
    printf("vh_knob");
}

void vh_knob_add(view_t* view, void (*ratio_changed)(view_t* view, float ratio))
{
    assert(view->handler == NULL && view->handler_data == NULL);

    vh_knob_t* vh = CAL(sizeof(vh_knob_t), NULL, vh_knob_desc);

    vh->ratio_changed = ratio_changed;

    view->handler_data = vh;
    view->handler      = vh_knob_evt;
}

#endif
