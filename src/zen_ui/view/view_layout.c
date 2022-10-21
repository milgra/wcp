#ifndef view_layout_h
#define view_layout_h

#include "view.c"

void view_layout(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <limits.h>
#include <math.h>

void view_layout(view_t* view)
{
    float act_x = 0;
    float act_y = 0;
    float rel_w = view->frame.local.w; // remaining width for relative views
    float rel_h = view->frame.local.h; // remaining height for relative views
    int   rem_w = view->views->length; // remaining relative views for width
    int   rem_h = view->views->length; // remaining relative views for height

    if (view->style.display == LD_FLEX)
    {
	if (view->style.flexdir == FD_ROW)
	{
	    for (int i = 0; i < view->views->length; i++)
	    {
		view_t* v = view->views->data[i];
		if (v->style.width > 0)
		{
		    rel_w -= v->style.width;
		    rem_w -= 1;
		}
		v->frame.local.x = 0;
		v->frame.local.y = 0;
	    }
	}
	if (view->style.flexdir == FD_COL)
	{
	    for (int i = 0; i < view->views->length; i++)
	    {
		view_t* v = view->views->data[i];
		if (v->style.height > 0)
		{
		    rel_h -= v->style.height;
		    rem_h -= 1;
		}
		v->frame.local.x = 0;
		v->frame.local.y = 0;
	    }
	}
    }

    for (int i = 0; i < view->views->length; i++)
    {
	view_t* v     = view->views->data[i];
	r2_t    frame = v->frame.local;

	if (v->style.margin == INT_MAX)
	{
	    frame.x = 0;
	    frame.y = 0;
	}

	if (v->style.margin_top < INT_MAX || v->style.margin_bottom < INT_MAX)
	{
	    frame.y = 0;
	}

	if (v->style.margin_left < INT_MAX || v->style.margin_right < INT_MAX)
	{
	    frame.x = 0;
	}

	if (v->style.width > 0)
	{
	    frame.w = v->style.width;
	    if (view->style.display == LD_FLEX && view->style.flexdir == FD_ROW)
	    {
		frame.x = act_x;
		act_x += frame.w;
	    }
	}
	if (v->style.height > 0)
	{
	    frame.h = v->style.height;
	    if (view->style.display == LD_FLEX && view->style.flexdir == FD_COL)
	    {
		frame.y = act_y;
		act_y += frame.h;
	    }
	}
	if (v->style.w_per > 0.0)
	{
	    float width = rel_w;
	    if (view->style.display == LD_FLEX && view->style.flexdir == FD_ROW)
	    {
		width   = floorf(rel_w / rem_w);
		frame.x = act_x;
		act_x += width;
		rem_w -= 1;
		rel_w -= width;
	    }
	    frame.w = width * v->style.w_per;
	}
	if (v->style.h_per > 0.0)
	{
	    float height = rel_h;
	    if (view->style.display == LD_FLEX && view->style.flexdir == FD_COL)
	    {
		height  = floorf(rel_h / rem_h);
		frame.y = act_y;
		act_y += height;
		rem_h -= 1;
		rel_h -= height;
	    }

	    frame.h = height * v->style.h_per;
	}
	if (v->style.margin == INT_MAX || view->style.cjustify == JC_CENTER)
	{
	    frame.x = (view->frame.local.w / 2.0) - (frame.w / 2.0);
	}
	if (view->style.itemalign == IA_CENTER)
	{
	    frame.y = (view->frame.local.h / 2.0) - (frame.h / 2.0);
	}
	if (v->style.margin_top < INT_MAX)
	{
	    frame.y += v->style.margin_top;
	    frame.h -= v->style.margin_top;
	}
	if (v->style.margin_left < INT_MAX)
	{
	    frame.x += v->style.margin_left;
	    frame.w -= v->style.margin_left;
	}
	if (v->style.margin_right < INT_MAX)
	{
	    frame.w -= v->style.margin_right;
	}
	if (v->style.margin_bottom < INT_MAX)
	{
	    frame.h -= v->style.margin_bottom;
	}
	if (v->style.top < INT_MAX)
	{
	    frame.y = v->style.top;
	}
	if (v->style.left < INT_MAX)
	{
	    frame.x = v->style.left;
	}
	if (v->style.right < INT_MAX)
	{
	    frame.x = view->frame.local.w - frame.w - v->style.right;
	}
	if (v->style.bottom < INT_MAX)
	{
	    frame.y = view->frame.local.h - frame.h - v->style.bottom;
	}
	view_set_frame(v, frame);
    }

    for (int i = 0; i < view->views->length; i++)
    {
	view_t* v = view->views->data[i];
	view_layout(v);
    }
}

#endif
