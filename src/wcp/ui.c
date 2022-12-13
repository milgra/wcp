#ifndef ui_h
#define ui_h

#include "ku_connector_wayland.c"
#include "ku_window.c"

void ui_init(int width, int height, float scale, ku_window_t* window, wl_window_t* wlwindow);
void ui_destroy();
void ui_load_values();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "ku_bitmap.c"
#include "ku_bitmap_ext.c"
#include "ku_draw.c"
#include "ku_gen_css.c"
#include "ku_gen_html.c"
#include "ku_gen_type.c"
#include "ku_text.c"
#include "ku_view.c"
#include "mt_log.c"
#include "mt_map_ext.c"
#include "mt_number.c"
#include "mt_path.c"
#include "mt_string.c"
#include "mt_time.c"
#include "tg_css.c"
#include "tg_knob.c"
#include "tg_text.c"
#include "vh_button.c"
#include "vh_key.c"
#include "vh_knob.c"
#include "vh_slider.c"
#include "vh_textinput.c"
#include "vh_touch.c"
#include <pthread.h>
#include <unistd.h>

struct _ui_t
{
    wl_window_t*    wlwindow;
    ku_view_t*      view_base;
    mt_vector_t*    view_list;
    char*           command;
    pthread_t       thread;
    ku_window_t*    window;
    pthread_cond_t  comm_cond;
    pthread_mutex_t comm_mutex;
} ui;

int ui_execute_command(char* command, char** result)
{
    char buff[100];

    FILE* pipe = popen(command, "r"); // CLOSE 0
    while (fgets(buff, sizeof(buff), pipe) != NULL) *result = mt_string_append(*result, buff);
    pclose(pipe); // CLOSE 0

    return 0;
}

void* ui_command_thread(void* data)
{
    while (1)
    {
	int r;
	if ((r = pthread_mutex_lock(&ui.comm_mutex)) != 0)
	{
	    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
	    exit(1);
	}

	pthread_cond_wait(&ui.comm_cond, &ui.comm_mutex);

	if (ui.command)
	{
	    char* result = mt_string_new_cstring("");
	    ui_execute_command(ui.command, &result);

	    REL(ui.command);
	    REL(result);
	    ui.command = NULL;
	}

	if ((r = pthread_mutex_unlock(&ui.comm_mutex)) != 0)
	{
	    fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
	    exit(1);
	}
    }

    return NULL;
}

void ui_on_key_down(void* userdata, void* data)
{
}

void ui_on_button_event(vh_button_event_t event)
{
    if (!ui.command)
    {
	char* command = mt_string_new_format(200, "sh %s/%s 1", config_get("scr_path"), event.view->script);
	ui.command    = command;
	pthread_cond_signal(&ui.comm_cond);
    }

    /* ku_wayland_delete_window(ui.wlwindow); */
}

void ui_on_slider_event(vh_slider_event_t event)
{
    int ratio = (int) (event.ratio * 100.0);

    if (!ui.command)
    {
	char* command = mt_string_new_format(200, "sh %s/%s %i", config_get("scr_path"), event.view->script, ratio);
	ui.command    = command;
	pthread_cond_signal(&ui.comm_cond);
    }
}

void ui_load_values()
{
    for (int index = 0; index < ui.view_list->length; index++)
    {
	ku_view_t* view = ui.view_list->data[index];

	if (view->script)
	{
	    char* command = mt_string_new_format(200, "sh %s/%s", config_get("scr_path"), view->script);
	    char* result  = mt_string_new_cstring("");
	    ui_execute_command(command, &result);

	    if (strcmp(view->type, "label") == 0)
	    {
		tg_text_set1(view, result);
	    }
	    if (strcmp(view->type, "slider") == 0)
	    {
		float ratio = (float) atoi(result) / 100.0;
		vh_slider_set(view, ratio);
	    }
	    REL(result);
	    REL(command);
	}
    }
}

void ui_init(int width, int height, float scale, ku_window_t* window, wl_window_t* wlwindow)
{
    ku_text_init();

    ui.window    = window;
    ui.wlwindow  = wlwindow;
    ui.view_list = VNEW();

    pthread_cond_init(&ui.comm_cond, NULL);
    pthread_mutex_init(&ui.comm_mutex, NULL);

    pthread_create(&ui.thread, NULL, &ui_command_thread, NULL);

    /* generate views from descriptors */

    ku_gen_html_parse(config_get("html_path"), ui.view_list);
    ku_gen_css_apply(ui.view_list, config_get("css_path"), config_get("img_path"));
    ku_gen_type_apply(ui.view_list, ui_on_button_event, ui_on_slider_event);

    ui.view_base = mt_vector_head(ui.view_list);

    /* initial layout of views */

    ku_view_set_frame(ui.view_base, (ku_rect_t){0.0, 0.0, (float) width, (float) height});
    ku_window_add(ui.window, ui.view_base);

    /* load values from scripts */

    ui_load_values();

    // show texture map for debug

    /* ku_view_t* texmap       = ku_view_new("texmap", ((r2_t){0, 0, 150, 150})); */
    /* texmap->needs_touch  = 0; */
    /* texmap->exclude      = 0; */
    /* texmap->texture.full = 1; */
    /* texmap->style.right  = 0; */
    /* texmap->style.top    = 0; */

    /* ui_manager_add(texmap); */
}

void ui_destroy()
{
    ku_window_remove(ui.window, ui.view_base);

    REL(ui.view_list);

    ku_text_destroy();
}

#endif
