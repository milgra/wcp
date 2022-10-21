#ifndef viewgen_type_h
#define viewgen_type_h

#include "vh_button.c"
#include "vh_slider.c"
#include "zc_vector.c"

void viewgen_type_apply(vec_t* views, void (*button_event)(vh_button_event_t), void (*slider_event)(vh_slider_event_t));

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"
#include "tg_text.c"
#include "view.c"

void viewgen_type_apply(vec_t* views, void (*button_event)(vh_button_event_t), void (*slider_event)(vh_slider_event_t))
{
    for (int index = 0; index < views->length; index++)
    {
	view_t* view = views->data[index];

	if (view->type && strcmp(view->type, "label") == 0)
	{
	    tg_text_add(view);
	}
	else if (view->style.background_color > 0)
	{
	    tg_css_add(view);
	}
	else if (view->style.background_image)
	{
	    tg_css_add(view);
	}

	if (view->type && strcmp(view->type, "button") == 0)
	{
	    vh_button_add(view, VH_BUTTON_NORMAL, button_event);
	}
	else if (view->type && strcmp(view->type, "slider") == 0)
	{
	    vh_slider_add(view, slider_event);
	}
    }
}

#endif
