#ifndef viewgen_type_h
#define viewgen_type_h

#include "zc_callback.c"
#include "zc_vector.c"

void viewgen_type_apply(vec_t* views, cb_t* callback);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_slider.c"
#include "view.c"

void viewgen_type_apply(vec_t* views, cb_t* callback)
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
	    printf("ADDING BACK IMAGE\n");
	    tg_css_add(view);
	}

	if (view->type && strcmp(view->type, "button") == 0)
	{
	    vh_button_add(view, VH_BUTTON_NORMAL, callback);
	}
	else if (view->type && strcmp(view->type, "slider") == 0)
	{
	    vh_slider_add(view, callback);
	}
    }
}

#endif
