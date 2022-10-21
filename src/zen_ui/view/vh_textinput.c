#ifndef vh_textinput_h
#define vh_textinput_h

#include "view.c"
#include "wm_event.c"
#include "zc_text.c"

#ifndef SDLK_BACKSPACE
    #define SDLK_BACKSPACE 0
#endif

#ifndef SDLK_RETURN
    #define SDLK_RETURN 0
#endif

#ifndef SDLK_ESCAPE
    #define SDLK_ESCAPE 0
#endif

typedef struct _vh_textinput_t vh_textinput_t;

enum vh_textinput_event_id
{
    VH_TEXTINPUT_TEXT,
    VH_TEXTINPUT_RETURN,
    VH_TEXTINPUT_ACTIVATE,
    VH_TEXTINPUT_DEACTIVATE
};

typedef struct _vh_textinput_event_t
{
    enum vh_textinput_event_id id;
    vh_textinput_t*            vh;
    char*                      text;
    view_t*                    view;
} vh_textinput_event_t;

struct _vh_textinput_t
{
    char*   text;     // text string
    vec_t*  glyph_v;  // glpyh views
    view_t* cursor_v; // cursor view
    view_t* holder_v; // placeholder text view
    r2_t    frame_s;  // starting frame for autosize minimal values

    int         glyph_index;
    textstyle_t style;
    char        active;

    int mouse_out_deact;

    void (*on_event)(vh_textinput_event_t);
};

void vh_textinput_add(view_t* view, char* text, char* phtext, textstyle_t textstyle, void (*on_event)(vh_textinput_event_t));

char* vh_textinput_get_text(view_t* view);
void  vh_textinput_set_text(view_t* view, char* text);
void  vh_textinput_set_deactivate_on_mouse_out(view_t* view, int flag);
void  vh_textinput_activate(view_t* view, char state);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "tg_css.c"
#include "tg_text.c"
#include "utf8.h"
#include "vh_anim.c"
#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_vector.c"

glyph_t* vh_textinput_glyphs_from_string(char* text, size_t* count)
{
    const void*  part   = text;
    size_t       length = utf8len(text);
    glyph_t*     glyphs = malloc(sizeof(glyph_t) * length); // REL 0
    utf8_int32_t cp;

    for (int i = 0; i < length; i++)
    {
	part         = utf8codepoint(part, &cp);
	glyphs[i].cp = cp;
    }

    *count = length;

    return glyphs;
}

void vh_textinput_upd(view_t* view)
{
    vh_textinput_t* data  = view->handler_data;
    r2_t            frame = view->frame.local;

    if (strlen(data->text) > 0)
    {
	size_t   count  = 0;
	glyph_t* glyphs = vh_textinput_glyphs_from_string(data->text, &count);

	int nw;
	int nh;

	text_layout(glyphs, count, data->style, frame.w, frame.h, &nw, &nh);

	// resize frame if needed
	if (data->style.autosize == AS_AUTO)
	{
	    frame = view->frame.local;
	    if (nw > frame.w)
	    {
		frame.w = nw;
		view_set_frame(view, frame);
	    }
	    if (nw <= frame.w)
	    {
		if (nw <= data->frame_s.w) nw = data->frame_s.w;
		frame.w = nw;
		view_set_frame(view, frame);
	    }
	    if (nh > frame.h)
	    {
		frame.h = nh;
		view_set_frame(view, frame);
	    }
	    if (nh <= frame.h)
	    {
		if (nh <= data->frame_s.h) nh = data->frame_s.h;
		frame.h = nh;
		view_set_frame(view, frame);
	    }
	}

	for (int i = 0; i < count; i++)
	{
	    glyph_t g = glyphs[i];

	    if (i < data->glyph_v->length)
	    {
		view_t* gv = data->glyph_v->data[i];

		if (g.w > 0 && g.h > 0)
		{
		    r2_t f  = gv->frame.local;
		    r2_t nf = (r2_t){g.x, g.y, g.w, g.h};

		    if (f.w == 0 || f.h == 0)
		    {
			bm_rgba_t* texture = bm_rgba_new(g.w, g.h); // REL 0

			text_render_glyph(g, data->style, texture);

			view_set_texture_bmp(gv, texture);

			REL(texture);

			gv->exclude = 0; // do we have to have 0 as default?!?!

			view_add_subview(view, gv);

			view_set_frame(gv, nf);

			// open
			r2_t sf = nf;
			sf.x    = 0.0;
			sf.y    = 0.0;
			nf.x    = 0.0;
			nf.y    = 0.0;
			sf.w    = 0.0;

			vh_anim_region(gv, sf, nf, 10, AT_EASE);

			view_set_region(gv, sf);
		    }
		    else
		    {
			r2_t rf = nf;
			rf.x    = 0;
			rf.y    = 0;
			view_set_region(gv, rf);

			if (!r2_equals(rf, gv->frame.region))
			{
			    vh_anim_finish(gv);
			    vh_anim_frame(gv, gv->frame.local, nf, 10, AT_EASE);
			}
		    }
		}
	    }
	    else
		printf("glyph and string count mismatch\n");
	}

	// update cursor position

	glyph_t last = {0};
	if (count > 0) last = glyphs[count - 1];

	r2_t crsr_f = {0};
	crsr_f.x    = last.x + last.w + 1;
	crsr_f.y    = last.base_y - last.asc - last.desc / 2.0;
	crsr_f.w    = 2;
	crsr_f.h    = last.asc;

	vh_anim_finish(data->cursor_v);
	vh_anim_frame(data->cursor_v, data->cursor_v->frame.local, crsr_f, 10, AT_EASE);

	// view_set_frame(data->cursor_v, crsr_f);

	free(glyphs); // REL 0
    }
    else
    {
	// move cursor to starting position based on textstyle

	// get line height
	glyph_t glyph;
	glyph.cp = ' ';
	int nw;
	int nh;
	text_layout(&glyph, 1, data->style, frame.w, frame.h, &nw, &nh);

	r2_t crsr_f = {0};
	crsr_f.w    = 2;
	crsr_f.h    = glyph.asc;

	if (data->style.align == TA_LEFT)
	{
	    crsr_f.x = data->style.margin_left;
	    crsr_f.y = data->style.margin || data->style.margin_top;
	    if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
	    if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
	}
	if (data->style.align == TA_RIGHT)
	{
	    crsr_f.x = frame.w - data->style.margin_right - crsr_f.w;
	    crsr_f.y = data->style.margin_top;
	    if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
	    if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
	}
	if (data->style.align == TA_CENTER)
	{
	    crsr_f.x = frame.w / 2 - crsr_f.w / 2;
	    crsr_f.y = data->style.margin_top;
	    if (data->style.valign == VA_CENTER) crsr_f.y = frame.h / 2 - crsr_f.h / 2;
	    if (data->style.valign == VA_BOTTOM) crsr_f.y = frame.h - data->style.margin_bottom - crsr_f.h;
	}

	vh_anim_finish(data->cursor_v);
	vh_anim_frame(data->cursor_v, data->cursor_v->frame.local, crsr_f, 10, AT_EASE);
    }

    // textinput_render_glyphs(glyphs, text->length, style, bitmap);
    // vh_anim_add(glyphview);
    // vh_anim_set(glyphview, sf, ef, 10, AT_LINEAR);

    // show text as texture
    // char* cstr = str_new_cstring(text);
    // tg_textet(view, cstr, data->style);
    // REL(cstr);
}

void vh_textinput_activate(view_t* view, char state)
{
    assert(view && view->handler_data != NULL);

    vh_textinput_t* data = view->handler_data;

    if (state)
    {
	if (!data->active)
	{
	    data->active = 1;

	    if (strlen(data->text) == 0)
	    {
		vh_anim_alpha(data->holder_v, 1.0, 0.0, 10, AT_LINEAR);
	    }
	    vh_anim_alpha(data->cursor_v, 0.0, 1.0, 10, AT_LINEAR);
	}
    }
    else
    {
	if (data->active)
	{
	    data->active = 0;

	    if (strlen(data->text) == 0)
	    {
		vh_anim_alpha(data->holder_v, 0.0, 1.0, 10, AT_LINEAR);
	    }
	    vh_anim_alpha(data->cursor_v, 1.0, 0.0, 10, AT_LINEAR);
	}
    }

    vh_textinput_upd(view);
}

void vh_textinput_on_anim(vh_anim_event_t event)
{
    view_t* tiview = event.userdata;

    vh_textinput_t* data = tiview->handler_data;

    if (vec_index_of_data(data->glyph_v, event.view) == UINT32_MAX) view_remove_from_parent(event.view);
}

void vh_textinput_evt(view_t* view, ev_t ev)
{
    vh_textinput_t* data = view->handler_data;
    if (ev.type == EV_TIME)
    {
    }
    else if (ev.type == EV_MDOWN)
    {
	// r2_t frame = view->frame.global;

	vh_textinput_activate(view, 1);

	vh_textinput_event_t event = {.id = VH_TEXTINPUT_ACTIVATE, .vh = data, .text = data->text, .view = view};
	if (data->on_event) (*data->on_event)(event);
    }
    else if (ev.type == EV_MDOWN_OUT)
    {
	if (data->mouse_out_deact)
	{
	    r2_t frame = view->frame.global;

	    if (ev.x < frame.x ||
		ev.x > frame.x + frame.w ||
		ev.y < frame.y ||
		ev.y > frame.y + frame.h)
	    {
		vh_textinput_activate(view, 0);
		vh_textinput_event_t event = {.id = VH_TEXTINPUT_DEACTIVATE, .vh = data, .text = data->text, .view = view};
		if (data->on_event) (*data->on_event)(event);
	    }
	}
    }
    else if (ev.type == EV_TEXT)
    {
	data->text = cstr_append(data->text, ev.text);

	// create view for glyph

	char view_id[100];
	snprintf(view_id, 100, "%sglyph%i", view->id, data->glyph_index++);
	view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0}); // REL 0

	vh_anim_add(glyph_view, vh_textinput_on_anim, view);
	glyph_view->texture.resizable = 0;

	VADD(data->glyph_v, glyph_view);

	REL(glyph_view); // REL 0

	// append or break-insert new glyph(s)

	vh_textinput_upd(view);

	vh_textinput_event_t event = {.id = VH_TEXTINPUT_TEXT, .vh = data, .text = data->text, .view = view};
	if (data->on_event) (*data->on_event)(event);
    }
    else if (ev.type == EV_KDOWN)
    {
	if (ev.keycode == SDLK_BACKSPACE && utf8len(data->text) > 0)
	{
	    size_t count = utf8len(data->text);
	    /* const void*  part  = data->text; */
	    /* utf8_int32_t cp; */
	    /* char*        new_text = CAL(strlen(data->text), NULL, NULL); */
	    /* char*        new_part = new_text; */

	    /* // remove last codepoint */
	    /* for (int index = 0; index < count - 1; index++) */
	    /* { */
	    /* 	part     = utf8codepoint(part, &cp); */
	    /* 	new_part = utf8catcodepoint(new_part, cp, 4); */
	    /* } */

	    char* new_text = cstr_new_delete_utf_codepoints(data->text, count - 1, 1);
	    if (data->text) REL(data->text);
	    data->text = new_text;

	    view_t* glyph_view = vec_tail(data->glyph_v);
	    VREM(data->glyph_v, glyph_view);

	    r2_t sf = glyph_view->frame.local;
	    r2_t ef = sf;
	    sf.x    = 0.0;
	    sf.y    = 0.0;
	    ef.x    = 0.0;
	    ef.y    = 0.0;
	    ef.w    = 0.0;

	    vh_anim_region(glyph_view, sf, ef, 10, AT_EASE);

	    vh_textinput_upd(view);

	    vh_textinput_event_t event = {.id = VH_TEXTINPUT_TEXT, .vh = data, .text = data->text, .view = view};
	    if (data->on_event) (*data->on_event)(event);
	}
	if (ev.keycode == SDLK_RETURN)
	{
	    vh_textinput_event_t event = {.id = VH_TEXTINPUT_RETURN, .vh = data, .text = data->text, .view = view};
	    if (data->on_event) (*data->on_event)(event);
	}
	if (ev.keycode == SDLK_ESCAPE)
	{
	    vh_textinput_activate(view, 0);

	    vh_textinput_event_t event = {.id = VH_TEXTINPUT_DEACTIVATE, .vh = data, .text = data->text, .view = view};
	    if (data->on_event) (*data->on_event)(event);
	}
    }
    else if (ev.type == EV_TIME)
    {
	// animate glyphs
    }
}

void vh_textinput_del(void* p)
{
    vh_textinput_t* vh = p;
    REL(vh->text);
    REL(vh->glyph_v);
    REL(vh->cursor_v);
    REL(vh->holder_v);
}

void vh_textinput_desc(void* p, int level)
{
    printf("vh_textinput");
}

void vh_textinput_add(view_t* view, char* text, char* phtext, textstyle_t textstyle, void (*on_event)(vh_textinput_event_t))
{
    assert(view->handler == NULL && view->handler_data == NULL);

    char* id_c = cstr_new_format(100, "%s%s", view->id, "crsr");   // REL 0
    char* id_h = cstr_new_format(100, "%s%s", view->id, "holder"); // REL 1

    vh_textinput_t* data = CAL(sizeof(vh_textinput_t), vh_textinput_del, vh_textinput_desc);

    textstyle.backcolor = 0;

    data->text    = cstr_new_cstring(""); // REL 2
    data->glyph_v = VNEW();               // REL 3

    data->style = textstyle;

    data->on_event = on_event;

    data->frame_s = view->frame.local;

    data->mouse_out_deact = 1;

    view->needs_key  = 1; // backspace event
    view->needs_text = 1; // unicode text event
    view->blocks_key = 1;

    view->handler      = vh_textinput_evt;
    view->handler_data = data;

    // cursor

    data->cursor_v                         = view_new(id_c, (r2_t){50, 12, 2, 0}); // REL 4
    data->cursor_v->style.background_color = 0x666666FF;

    tg_css_add(data->cursor_v);
    vh_anim_add(data->cursor_v, NULL, NULL);

    view_set_texture_alpha(data->cursor_v, 0.0, 1);
    view_add_subview(view, data->cursor_v);

    // placeholder

    textstyle_t phts = textstyle;
    phts.align       = TA_CENTER;
    phts.textcolor   = 0x888888FF;

    data->holder_v              = view_new(id_h, (r2_t){0, 0, view->frame.local.w, view->frame.local.h}); // REL 0
    data->holder_v->style.w_per = 1.0;
    data->holder_v->style.h_per = 1.0;

    tg_text_add(data->holder_v);
    tg_text_set(data->holder_v, phtext, phts);

    vh_anim_add(data->holder_v, NULL, NULL);

    data->holder_v->blocks_touch = 0;

    view_add_subview(view, data->holder_v);

    // view setup

    // tg_text_add(view);

    // add placeholder view

    // text_s setup

    if (text)
    {
	data->text = cstr_append(data->text, text);

	for (int i = 0; i < utf8len(data->text); i++)
	{
	    char view_id[100];
	    snprintf(view_id, 100, "%s_glyph_%i", view->id, data->glyph_index++);

	    view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0}); // REL 0
	    vh_anim_add(glyph_view, vh_textinput_on_anim, view);

	    VADD(data->glyph_v, glyph_view);

	    REL(glyph_view); // REL 0
	}
    }

    // update text

    vh_textinput_upd(view);

    // cleanup

    REL(id_c);
    REL(id_h);
}

void vh_textinput_set_text(view_t* view, char* text)
{
    vh_textinput_t* data = view->handler_data;

    // remove glyphs

    for (int i = data->glyph_v->length - 1; i > -1; i--)
    {
	view_t* gv = data->glyph_v->data[i];

	view_remove_from_parent(gv);

	/* r2_t sf = gv->frame.local; */
	/* r2_t ef = sf; */
	/* sf.x    = 0.0; */
	/* sf.y    = 0.0; */
	/* ef.x    = 0.0; */
	/* ef.y    = 0.0; */
	/* ef.w    = 0.0; */

	/* vh_anim_region(gv, sf, ef, 10 + i, AT_EASE); */
	/* vh_anim_set_event(gv, view, vh_textinput_on_glyph_close); */
    }
    vec_reset(data->glyph_v);

    // text_s setup
    // TODO create function from this to reuse

    if (text)
    {
	if (data->text) REL(data->text);
	data->text = cstr_new_cstring(text);

	for (int i = 0; i < utf8len(data->text); i++)
	{
	    char view_id[100];
	    snprintf(view_id, 100, "%sglyph%i", view->id, data->glyph_index++);
	    view_t* glyph_view = view_new(view_id, (r2_t){0, 0, 0, 0}); // REL 1
	    vh_anim_add(glyph_view, vh_textinput_on_anim, view);

	    VADD(data->glyph_v, glyph_view);

	    REL(glyph_view); // REL 1
	}

	vh_anim_alpha(data->holder_v, 1.0, 0.0, 10, AT_LINEAR);
    }

    vh_textinput_upd(view);

    vh_textinput_event_t event = {.id = VH_TEXTINPUT_TEXT, .vh = data, .text = data->text, .view = view};
    if (data->on_event) (*data->on_event)(event);
}

char* vh_textinput_get_text(view_t* view)
{
    vh_textinput_t* data = view->handler_data;
    return data->text;
}

void vh_textinput_set_deactivate_on_mouse_out(view_t* view, int flag)
{
    vh_textinput_t* data  = view->handler_data;
    data->mouse_out_deact = flag;
}

#endif
