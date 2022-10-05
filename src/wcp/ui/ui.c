#ifndef ui_h
#define ui_h

#include "view.c"

void ui_init(float width, float height);
void ui_post_render_init();
void ui_destroy();

void ui_add_cursor();
void ui_update_cursor(r2_t frame);
void ui_screenshot(uint32_t time);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "bm_rgba_util.c"
#include "config.c"
#include "map_util.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_text.c"
#include "ui_compositor.c"
#include "ui_manager.c"
#include "ui_table.c"
#include "ui_util.c"
#include "vh_button.c"
#include "vh_key.c"
#include "vh_knob.c"
#include "vh_slider.c"
#include "vh_textinput.c"
#include "vh_touch.c"
#include "view_layout.c"
#include "viewgen_css.c"
#include "viewgen_html.c"
#include "viewgen_type.c"
#include "zc_bm_rgba.c"
#include "zc_callback.c"
#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_number.c"
#include "zc_path.c"
#include "zc_text.c"
#include "zc_time.c"

struct _ui_t
{
    view_t* cursor;
    view_t* view_base;
    vec_t*  view_list;

    view_t* seekknob;
    view_t* volknob;
} ui;

int execute_command(char* command, char** result)
{
    char buff[100];

    FILE* pipe = popen(command, "r"); // CLOSE 0
    while (fgets(buff, sizeof(buff), pipe) != NULL) *result = cstr_append(*result, buff);
    pclose(pipe); // CLOSE 0

    return 0;
}

void ui_on_key_down(void* userdata, void* data)
{
}

void ui_on_event(void* userdata, void* data)
{
    view_t* view = data;

    if (view->type && strcmp(view->type, "slider") == 0)
    {
	int ratio = (int) (vh_slider_get_ratio(view) * 100.0);

	char* script  = path_new_append(config_get("res_path"), "script/");
	char* command = cstr_new_format(200, "bash %s%s %i", script, view->script, ratio);
	char* result  = cstr_new_cstring("");
	execute_command(command, &result);
    }

    if (view->type && strcmp(view->type, "button") == 0)
    {
	char* script  = path_new_append(config_get("res_path"), "script/");
	char* command = cstr_new_format(200, "bash %s%s 1", script, view->script);
	char* result  = cstr_new_cstring("");
	execute_command(command, &result);
    }
}

void on_songlist_event(ui_table_t* table, ui_table_event event, void* userdata)
{
    switch (event)
    {
	case UI_TABLE_EVENT_FIELDS_UPDATE:
	{
	}
	break;
	case UI_TABLE_EVENT_SELECT:
	{
	}
	break;
	case UI_TABLE_EVENT_OPEN:
	{
	    /* vec_t* selected = userdata; */
	    /* map_t* info     = selected->data[0]; */
	}
	break;
	case UI_TABLE_EVENT_DRAG:
	{
	}
	break;
	case UI_TABLE_EVENT_KEY:
	{
	    /* ev_t ev = *((ev_t*) userdata); */

	    /* if (ev.keycode == SDLK_DOWN || ev.keycode == SDLK_UP) */
	    /* { */
	    /* 	int32_t index = table->selected_index; */

	    /* 	if (ev.keycode == SDLK_DOWN) index += 1; */
	    /* 	if (ev.keycode == SDLK_UP) index -= 1; */
	    /* 	ui_table_select(table, index); */
	    /* } */
	}
	break;
	case UI_TABLE_EVENT_DROP:
	    break;
    }
}

void ui_init(float width, float height)
{
    zc_log_debug("ui init");

    text_init();                    // DESTROY 0
    ui_manager_init(width, height); // DESTROY 1

    /* generate views from descriptors */

    cb_t* btn_cb = cb_new(ui_on_event, NULL);

    ui.view_list = VNEW();

    viewgen_html_parse(config_get("html_path"), ui.view_list);
    viewgen_css_apply(ui.view_list, config_get("css_path"), config_get("res_path"));
    viewgen_type_apply(ui.view_list, btn_cb);

    ui.view_base = RET(vec_head(ui.view_list));

    /* initial layout of views */

    view_set_frame(ui.view_base, (r2_t){0.0, 0.0, (float) width, (float) height});
    ui_manager_add(ui.view_base);

    // show texture map for debug

    /* view_t* texmap       = view_new("texmap", ((r2_t){0, 0, 150, 150})); */
    /* texmap->needs_touch  = 0; */
    /* texmap->exclude      = 0; */
    /* texmap->texture.full = 1; */
    /* texmap->style.right  = 0; */
    /* texmap->style.top    = 0; */

    /* ui_manager_add(texmap); */
}

void ui_post_render_init()
{
    /* load values from scripts */

    for (int index = 0; index < ui.view_list->length; index++)
    {
	view_t* view = ui.view_list->data[index];

	if (view->script)
	{
	    char* script  = path_new_append(config_get("res_path"), "script/");
	    char* command = cstr_new_format(200, "bash %s%s", script, view->script);
	    char* result  = cstr_new_cstring("");
	    execute_command(command, &result);

	    printf("result for %s : %s\n", command, result);

	    if (strcmp(view->type, "label") == 0)
	    {
		textstyle_t ts = ui_util_gen_textstyle(view);
		tg_text_set(view, result, ts);
	    }
	    if (strcmp(view->type, "slider") == 0)
	    {
		float ratio = (float) atoi(result) / 100.0;
		vh_slider_set(view, ratio);
	    }
	}
    }
}

void ui_destroy()
{
    ui_manager_remove(ui.view_base);

    REL(ui.view_list);
    REL(ui.view_base);

    ui_manager_destroy(); // DESTROY 1

    text_destroy(); // DESTROY 0
}

void ui_add_cursor()
{
    ui.cursor                         = view_new("ui.cursor", ((r2_t){10, 10, 10, 10}));
    ui.cursor->exclude                = 0;
    ui.cursor->style.background_color = 0xFF000099;
    ui.cursor->needs_touch            = 0;
    tg_css_add(ui.cursor);
    ui_manager_add_to_top(ui.cursor);
}

void ui_update_cursor(r2_t frame)
{
    view_set_frame(ui.cursor, frame);
}

void ui_screenshot(uint32_t time)
{
    if (config_get("lib_path"))
    {
	static int cnt    = 0;
	view_t*    root   = ui_manager_get_root();
	r2_t       frame  = root->frame.local;
	bm_rgba_t* screen = bm_rgba_new(frame.w, frame.h); // REL 0

	// remove cursor for screenshot to remain identical

	if (ui.cursor)
	{
	    ui_manager_remove(ui.cursor);
	    ui_manager_render(time, NULL);
	    ui_manager_add_to_top(ui.cursor);
	}

	ui_compositor_render_to_bmp(screen);

	char*      name    = cstr_new_format(20, "screenshot%.3i.png", cnt++); // REL 1
	char*      path    = path_new_append(config_get("lib_path"), name);    // REL 2
	bm_rgba_t* flipped = bm_rgba_new_flip_y(screen);                       // REL 3

	/* coder_write_png(path, flipped); */

	REL(flipped); // REL 3
	REL(name);    // REL 2
	REL(path);    // REL 1
	REL(screen);  // REL 0

	if (ui.cursor) ui_update_cursor(frame); // full screen cursor to indicate screenshot, next step will reset it
    }
}

#endif
