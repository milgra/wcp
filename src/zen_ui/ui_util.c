#ifndef ui_util_h
#define ui_util_h

#include "view.c"
#include "zc_text.c"

textstyle_t ui_util_gen_textstyle(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "fontconfig.c"
#include "zc_log.c"
#include <limits.h>

textstyle_t ui_util_gen_textstyle(view_t* view)
{
    textstyle_t style = {0};

    char* font = fontconfig_new_path(view->style.font_family);
    strcpy(style.font, font);
    REL(font);

    style.size = view->style.font_size > 0 ? view->style.font_size : 15;

    style.align = view->style.text_align;
    /* style.valign = */
    /* style.autosize = */
    style.multiline   = view->style.word_wrap == 1;
    style.line_height = view->style.line_height;

    style.margin = view->style.margin;
    if (view->style.margin_left < INT_MAX) style.margin_left = view->style.margin_left;
    if (view->style.margin_right < INT_MAX) style.margin_right = view->style.margin_right;
    if (view->style.margin_top < INT_MAX) style.margin_top = view->style.margin_top;
    if (view->style.margin_bottom < INT_MAX) style.margin_bottom = view->style.margin_bottom;

    style.textcolor = view->style.color;
    style.backcolor = view->style.background_color;

    return style;
}

#endif
