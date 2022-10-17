#ifndef ui_h
#define ui_h

#include "view.c"

void ui_init(float width, float height, float scale);
void ui_destroy();
void ui_load_values();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "bm_rgba_util.c"
#include "config.c"
#include "map_util.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_text.c"
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
#include "wl_connector.c"
#include "zc_bm_rgba.c"
#include "zc_callback.c"
#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_number.c"
#include "zc_path.c"
#include "zc_text.c"
#include "zc_time.c"
#include <pthread.h>
#include <unistd.h>

struct _ui_t
{
    view_t*   view_base;
    vec_t*    view_list;
    char*     command;
    pthread_t thread;

} ui;

int ui_execute_command(char* command, char** result)
{
    char buff[100];

    FILE* pipe = popen(command, "r"); // CLOSE 0
    while (fgets(buff, sizeof(buff), pipe) != NULL) *result = cstr_append(*result, buff);
    pclose(pipe); // CLOSE 0

    return 0;
}

void* ui_command_thread(void* data)
{
    while (1)
    {
	if (ui.command)
	{
	    char* result = cstr_new_cstring("");
	    ui_execute_command(ui.command, &result);

	    REL(ui.command);
	    REL(result);
	    ui.command = NULL;
	}
	else usleep(100);
    }

    return NULL;
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

	if (!ui.command)
	{
	    char* script  = path_new_append(config_get("res_path"), "script/");
	    char* command = cstr_new_format(200, "sh %s%s %i", script, view->script, ratio);
	    ui.command    = command;
	    REL(script);
	}
    }

    if (view->type && strcmp(view->type, "button") == 0)
    {
	if (!ui.command)
	{
	    char* script  = path_new_append(config_get("res_path"), "script/");
	    char* command = cstr_new_format(200, "sh %s%s 1", script, view->script);
	    ui.command    = command;
	    REL(script);
	}
	wl_hide();
    }
}

void ui_load_values()
{
    for (int index = 0; index < ui.view_list->length; index++)
    {
	view_t* view = ui.view_list->data[index];

	if (view->script)
	{
	    char* script  = path_new_append(config_get("res_path"), "script/");
	    char* command = cstr_new_format(200, "sh %s%s", script, view->script);
	    char* result  = cstr_new_cstring("");
	    ui_execute_command(command, &result);

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
	    REL(script);
	    REL(result);
	    REL(command);
	}
    }
}

void ui_init(float width, float height, float scale)
{
    zc_log_debug("ui init");

    text_init();                    // DESTROY 0
    ui_manager_init(width, height); // DESTROY 1

    pthread_create(&ui.thread, NULL, &ui_command_thread, NULL);

    /* generate views from descriptors */

    cb_t* btn_cb = cb_new(ui_on_event, NULL);

    ui.view_list = VNEW();

    viewgen_html_parse(config_get("html_path"), ui.view_list);
    viewgen_css_apply(ui.view_list, config_get("css_path"), config_get("res_path"), scale);
    viewgen_type_apply(ui.view_list, btn_cb);

    ui.view_base = vec_head(ui.view_list);

    /* initial layout of views */

    view_set_frame(ui.view_base, (r2_t){0.0, 0.0, (float) width, (float) height});
    ui_manager_add(ui.view_base);

    /* load values from scripts */

    ui_load_values();

    /* cleanup  */

    REL(btn_cb);

    // show texture map for debug

    /* view_t* texmap       = view_new("texmap", ((r2_t){0, 0, 150, 150})); */
    /* texmap->needs_touch  = 0; */
    /* texmap->exclude      = 0; */
    /* texmap->texture.full = 1; */
    /* texmap->style.right  = 0; */
    /* texmap->style.top    = 0; */

    /* ui_manager_add(texmap); */
}

void ui_destroy()
{
    ui_manager_remove(ui.view_base);

    REL(ui.view_list);

    ui_manager_destroy(); // DESTROY 1

    text_destroy(); // DESTROY 0
}

#endif
