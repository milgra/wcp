#include "config.c"
#include "ku_bitmap.c"
#include "ku_connector_wayland.c"
#include "ku_draw.c"
#include "ku_event.c"
#include "ku_renderer_soft.c"
#include "ku_window.c"
#include "mt_log.c"
#include "mt_path.c"
#include "mt_time.c"
#include "ui.c"
#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

struct
{
    wl_window_t* wlwindow;
    ku_window_t* kuwindow;
    ku_rect_t    dirtyrect;

    int   width;
    int   height;
    int   margin;
    char* anchor;
} wcp = {0};

void init(wl_event_t event)
{
    struct monitor_info* monitor = event.monitors[0];

    /* wcp.wlwindow = ku_wayland_create_window("wcp", 1200, 600); */
    wcp.wlwindow = ku_wayland_create_generic_layer(monitor, wcp.width, wcp.height, wcp.margin, wcp.anchor, 0);
    wcp.kuwindow = ku_window_create(wcp.width * monitor->scale, wcp.height * monitor->scale, monitor->scale);

    ui_init(monitor->logical_width, monitor->logical_height, monitor->scale, wcp.kuwindow, wcp.wlwindow);
}

/* window update */

void update(ku_event_t ev)
{
    if (ev.type == KU_EVENT_STDIN)
    {
	if (ev.text[0] == '0' && wcp.wlwindow->hidden == 0) ku_wayland_hide_window(wcp.wlwindow);
	else if (ev.text[0] == '1' && wcp.wlwindow->hidden == 1)
	{
	    // TODO figure out something to force full dirty rect update after show
	    wcp.kuwindow->root->rearrange = 1;
	    ku_wayland_show_window(wcp.wlwindow);
	    ui_load_values();
	}
	else if (ev.text[0] == '2')
	{
	    if (wcp.wlwindow->hidden == 0) ku_wayland_hide_window(wcp.wlwindow);
	    else if (wcp.wlwindow->hidden == 1)
	    {
		ku_wayland_show_window(wcp.wlwindow);
		ui_load_values();
	    }
	}
	else if (ev.text[0] == '3') ku_wayland_exit();
    }

    ku_window_event(wcp.kuwindow, ev);

    if (wcp.wlwindow->frame_cb == NULL)
    {
	ku_rect_t dirty = ku_window_update(wcp.kuwindow, 0);

	if (dirty.w > 0 && dirty.h > 0)
	{
	    ku_rect_t sum = ku_rect_add(dirty, wcp.dirtyrect);

	    /* mt_log_debug("drt %i %i %i %i", (int) dirty.x, (int) dirty.y, (int) dirty.w, (int) dirty.h); */
	    /* mt_log_debug("drt prev %i %i %i %i", (int) wcp.dirtyrect.x, (int) wcp.dirtyrect.y, (int) wcp.dirtyrect.w, (int) wcp.dirtyrect.h); */
	    /* mt_log_debug("sum aftr %i %i %i %i", (int) sum.x, (int) sum.y, (int) sum.w, (int) sum.h); */

	    ku_renderer_software_render(wcp.kuwindow->views, &wcp.wlwindow->bitmap, sum);

	    ku_wayland_request_frame(wcp.wlwindow);
	    // TODO fix dirty in case of 2.0 scaling
	    ku_wayland_draw_window(wcp.wlwindow, 0, 0, wcp.wlwindow->width, wcp.wlwindow->height);

	    wcp.dirtyrect = dirty;
	}
    }
}

void destroy()
{
    ku_wayland_delete_window(wcp.wlwindow);
    ui_destroy();

    REL(wcp.anchor);
    REL(wcp.kuwindow);
}

int main(int argc, char* argv[])
{
    mt_log_use_colors(isatty(STDERR_FILENO));
    mt_log_level_info();
    mt_time(NULL);

    printf("Wayland Control Panel v" WCP_VERSION
	   " by Milan Toth ( www.milgra.com )\n"
	   "If you like this app try :\n"
	   "- Sway Oveview ( github.com/milgra/sov )\n"
	   "- Visual Music Player (github.com/milgra/vmp)\n"
	   "- Multimedia File Manager (github.com/milgra/mmfm)\n"
	   "- SwayOS (swayos.github.io)\n"
	   "Games :\n"
	   "- Brawl (github.com/milgra/brawl)\n"
	   "- Cortex ( github.com/milgra/cortex )\n"
	   "- Termite (github.com/milgra/termite)\n\n");

    const char* usage =
	"Usage: wcp [options]\n"
	"\n"
	"  -h, --help                            Show help message and quit.\n"
	"  -v, --verbose                         Increase verbosity of messages, defaults to errors and warnings only.\n"
	"  -a, --anchor=[lrtp]                   Anchor window to window edge in directions, use rt for right top\n"
	"  -f, --frame=[width]x[height]          Initial window dimension\n"
	"  -m, --margin=[size]                   Margin\n"
	"  -r, --resources=[resources folder]    Resources dir for current session\n"
	"\n";

    const struct option long_options[] =
	{
	    {"help", no_argument, NULL, 'h'},
	    {"verbose", no_argument, NULL, 'v'},
	    {"anchor", optional_argument, NULL, 'a'},
	    {"frame", optional_argument, NULL, 'f'},
	    {"margin", optional_argument, NULL, 'm'},
	    {"resources", optional_argument, 0, 'r'}};

    char* res_par = NULL;
    char* frm_par = NULL;
    char* mrg_par = NULL;
    char* anc_par = NULL;

    int verbose      = 0;
    int option       = 0;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "vhr:f:m:a:", long_options, &option_index)) != -1)
    {
	switch (option)
	{
	    case '?': printf("parsing option %c value: %s\n", option, optarg); break;
	    case 'a': anc_par = mt_string_new_cstring(optarg); break; // REL 0
	    case 'r': res_par = mt_string_new_cstring(optarg); break; // REL 0
	    case 'f': frm_par = mt_string_new_cstring(optarg); break; // REL 1
	    case 'm': mrg_par = mt_string_new_cstring(optarg); break; // REL 2
	    case 'v': verbose = 1; break;
	    default: fprintf(stderr, "%s", usage); return EXIT_FAILURE;
	}
    }

    srand((unsigned int) time(NULL));

    char cwd[PATH_MAX] = {"~"};
    if (getcwd(cwd, sizeof(cwd)) == NULL) printf("Cannot get working directory\n");

    char* wrk_path = mt_path_new_normalize(cwd, NULL); // REL 3

    char* res_path     = NULL;
    char* res_path_loc = res_par ? mt_path_new_normalize(res_par, wrk_path) : mt_path_new_normalize("~/.config/wcp", getenv("HOME")); // REL 4
    char* res_path_glo = mt_string_new_cstring(PKG_DATADIR);                                                                          // REL 5

    DIR* dir = opendir(res_path_loc);
    if (dir)
    {
	res_path = res_path_loc;
	closedir(dir);
    }
    else res_path = res_path_glo;

    char* css_path  = mt_path_new_append(res_path, "html/main.css");  // REL 6
    char* html_path = mt_path_new_append(res_path, "html/main.html"); // REL 7
    char* img_path  = mt_path_new_append(res_path, "img");            // REL 6
    char* scr_path  = mt_path_new_append(res_path, "script");         // REL 8

    // print path info to console

    printf("working path  : %s\n", wrk_path);
    printf("resource path : %s\n", res_path);
    printf("css path      : %s\n", css_path);
    printf("html path     : %s\n", html_path);
    printf("image path    : %s\n", img_path);
    printf("script path   : %s\n", scr_path);
    printf("\n");

    if (verbose) mt_log_inc_verbosity();

    // init config

    config_init(); // DESTROY 0
    config_set("res_path", res_path);

    // init non-configurable defaults

    config_set("wrk_path", wrk_path);
    config_set("css_path", css_path);
    config_set("html_path", html_path);
    config_set("scr_path", scr_path);
    config_set("img_path", img_path);

    wcp.width  = 300;
    wcp.height = 300;
    wcp.margin = 0;

    if (anc_par) wcp.anchor = anc_par;
    else wcp.anchor = "";

    if (frm_par != NULL)
    {
	wcp.width  = atoi(frm_par);
	char* next = strstr(frm_par, "x");
	wcp.height = atoi(next + 1);
    }

    if (mrg_par != NULL)
    {
	wcp.margin = atoi(mrg_par);
    }

    ku_wayland_init(init, update, destroy, 0);

    /* show in wayland buffer */

    config_destroy(); // DESTROY 0

    // cleanup

    if (res_par) REL(res_par); // REL 0
    if (frm_par) REL(frm_par); // REL 1
    if (mrg_par) REL(mrg_par); // REL 1

    REL(wrk_path);     // REL 3
    REL(res_path_loc); // REL 4
    REL(res_path_glo); // REL 5
    REL(css_path);     // REL 6
    REL(html_path);    // REL 7
    REL(scr_path);     // REL 8
    REL(img_path);

#ifdef MT_MEMORY_DEBUG
    mt_memory_stats();
#endif
}
