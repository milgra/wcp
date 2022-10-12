#ifndef viewgen_css_h
#define viewgen_css_h

#include "view.c"
#include "zc_vector.c"

void viewgen_css_apply(vec_t* views, char* csspath, char* respath, float scale);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "css.c"
#include "zc_log.c"
#include <limits.h>

void viewgen_css_apply_style(view_t* view, map_t* style, char* respath, float scale)
{
    vec_t* keys = VNEW(); // REL 0
    map_keys(style, keys);

    for (int index = 0; index < keys->length; index++)
    {
	char* key = keys->data[index];
	char* val = MGET(style, key);

	if (strcmp(key, "background-color") == 0)
	{
	    int color                    = (int) strtol(val + 1, NULL, 16);
	    view->style.background_color = color;
	}
	else if (strcmp(key, "background-image") == 0)
	{
	    if (strstr(val, "url") != NULL)
	    {
		char* url = CAL(sizeof(char) * strlen(val), NULL, cstr_describe); // REL 0
		memcpy(url, val + 5, strlen(val) - 7);
		char* imagepath              = cstr_new_format(100, "%s/img/%s", respath, url);
		view->style.background_image = imagepath;
		REL(url); // REL 0
	    }
	}
	else if (strcmp(key, "font-family") == 0)
	{
	    char* url = CAL(sizeof(char) * strlen(val), NULL, cstr_describe); // REL 0
	    memcpy(url, val + 1, strlen(val) - 2);
	    view->style.font_family = url;
	}
	else if (strcmp(key, "color") == 0)
	{
	    int color         = (int) strtol(val + 1, NULL, 16);
	    view->style.color = color;
	}
	else if (strcmp(key, "font-size") == 0)
	{
	    float size            = atof(val);
	    view->style.font_size = size * scale;
	}
	else if (strcmp(key, "line-height") == 0)
	{
	    float size              = atof(val);
	    view->style.line_height = size * scale;
	}
	else if (strcmp(key, "word-wrap") == 0)
	{
	    if (strstr(val, "normal") != NULL) view->style.word_wrap = 0;
	    if (strstr(val, "break-word") != NULL) view->style.word_wrap = 1;
	    if (strstr(val, "initial") != NULL) view->style.word_wrap = 2;
	    if (strstr(val, "inherit") != NULL) view->style.word_wrap = 3;
	}
	else if (strcmp(key, "text-align") == 0)
	{
	    if (strstr(val, "left") != NULL) view->style.text_align = 0;
	    if (strstr(val, "center") != NULL) view->style.text_align = 1;
	    if (strstr(val, "right") != NULL) view->style.text_align = 2;
	    if (strstr(val, "justify") != NULL) view->style.text_align = 3;
	}
	else if (strcmp(key, "width") == 0)
	{
	    if (strstr(val, "%") != NULL)
	    {
		int per           = atoi(val);
		view->style.w_per = (float) per / 100.0;
	    }
	    else if (strstr(val, "px") != NULL)
	    {
		int   pix           = atoi(val);
		float fpix          = (int) ((float) pix * scale);
		view->style.width   = fpix;
		view->frame.local.w = fpix;
	    }
	}
	else if (strcmp(key, "height") == 0)
	{
	    if (strstr(val, "%") != NULL)
	    {
		int per           = atoi(val);
		view->style.h_per = (float) per / 100.0;
	    }
	    else if (strstr(val, "px") != NULL)
	    {
		int   pix           = atoi(val);
		float fpix          = (int) ((float) pix * scale);
		view->style.height  = fpix;
		view->frame.local.h = fpix;
	    }
	}
	else if (strcmp(key, "display") == 0)
	{
	    if (strcmp(val, "flex") == 0)
	    {
		view->style.display = LD_FLEX;
		view->exclude       = 1;
	    }
	    if (strcmp(val, "none") == 0)
		view->exclude = 1;
	}
	else if (strcmp(key, "overflow") == 0)
	{
	    if (strcmp(val, "hidden") == 0)
		view_set_masked(view, 1);
	}
	else if (strcmp(key, "flex-direction") == 0)
	{
	    if (strcmp(val, "column") == 0)
		view->style.flexdir = FD_COL;
	    else
		view->style.flexdir = FD_ROW;
	}
	else if (strcmp(key, "margin") == 0)
	{
	    if (strcmp(val, "auto") == 0)
	    {
		view->style.margin = INT_MAX;
	    }
	    else if (strstr(val, "px") != NULL)
	    {
		int   pix                 = atoi(val);
		float fpix                = (int) ((float) pix * scale);
		view->style.margin        = fpix;
		view->style.margin_top    = fpix;
		view->style.margin_left   = fpix;
		view->style.margin_right  = fpix;
		view->style.margin_bottom = fpix;
	    }
	}
	else if (strcmp(key, "top") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix       = atoi(val);
		float fpix      = (int) ((float) pix * scale);
		view->style.top = fpix;
	    }
	}
	else if (strcmp(key, "left") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix        = atoi(val);
		float fpix       = (int) ((float) pix * scale);
		view->style.left = fpix;
	    }
	}
	else if (strcmp(key, "right") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix         = atoi(val);
		float fpix        = (int) ((float) pix * scale);
		view->style.right = fpix;
	    }
	}
	else if (strcmp(key, "bottom") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix          = atoi(val);
		float fpix         = (int) ((float) pix * scale);
		view->style.bottom = fpix;
	    }
	}
	else if (strcmp(key, "margin-top") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix              = atoi(val);
		float fpix             = (int) ((float) pix * scale);
		view->style.margin_top = fpix;
	    }
	}
	else if (strcmp(key, "margin-left") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix               = atoi(val);
		float fpix              = (int) ((float) pix * scale);
		view->style.margin_left = fpix;
	    }
	}
	else if (strcmp(key, "margin-right") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix                = atoi(val);
		float fpix               = (int) ((float) pix * scale);
		view->style.margin_right = fpix;
	    }
	}
	else if (strcmp(key, "margin-bottom") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix                 = atoi(val);
		float fpix                = (int) ((float) pix * scale);
		view->style.margin_bottom = fpix;
	    }
	}
	else if (strcmp(key, "border-radius") == 0)
	{
	    if (strstr(val, "px") != NULL)
	    {
		int   pix                 = atoi(val);
		float fpix                = (int) ((float) pix * scale);
		view->style.border_radius = fpix;
	    }
	}
	else if (strcmp(key, "box-shadow") == 0)
	{
	    view->style.shadow_blur = atoi(val);
	    char* color             = strstr(val + 1, " ");

	    if (color) view->style.shadow_color = (int) strtol(color + 2, NULL, 16);
	}
	else if (strcmp(key, "align-items") == 0)
	{
	    if (strcmp(val, "center") == 0)
	    {
		view->style.itemalign = IA_CENTER;
	    }
	}
	else if (strcmp(key, "justify-content") == 0)
	{
	    if (strcmp(val, "center") == 0)
	    {
		view->style.cjustify = JC_CENTER;
	    }
	}
	// TODO remove non standard CSS
	else if (strcmp(key, "blocks") == 0)
	{
	    if (strcmp(val, "no") == 0)
	    {
		view->blocks_touch = 0;
	    }
	}
    }
    /* printf("style for %s: ", view->id); */
    /* view_desc_style(view->style); */
    /* printf("\n"); */

    REL(keys);
}

void viewgen_css_apply(vec_t* views, char* csspath, char* respath, float scale)
{
    map_t* styles = css_new(csspath);
    map_t* style;

    for (int index = 0; index < views->length; index++)
    {
	view_t* view = views->data[index];

	// apply id selector
	char cssid[100] = {0};
	snprintf(cssid, 100, "#%s", view->id);

	style = MGET(styles, cssid);
	if (style)
	{
	    viewgen_css_apply_style(view, style, respath, scale);
	}

	if (view->class)
	{

	    // apply class selector
	    char csscls[100] = {0};
	    snprintf(csscls, 100, "%s", view->class);

	    // tokenize if there is multiple classes
	    char* token = strtok(csscls, " ");
	    do
	    {
		char cls[100] = {0};
		snprintf(cls, 100, ".%s", token);
		style = MGET(styles, cls);
		// zc_log_debug("applying class %s to %s", cls, view->id);
		if (style)
		{
		    viewgen_css_apply_style(view, style, respath, scale);
		}
	    } while ((token = strtok(NULL, " ")));
	}
    }

    REL(styles);
}

#endif
