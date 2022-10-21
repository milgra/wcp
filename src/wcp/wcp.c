#include "config.c"
#include "ui.c"
#include "ui_manager.c"
#include "wl_connector.c"
#include "zc_bm_rgba.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_path.c"
#include "zc_time.c"
#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void init(int width, int height, float scale)
{
    ui_init(width, height, scale); // DESTROY 3
}

void update(ev_t ev)
{
    if (ev.type == EV_WINDOW_SHOW) ui_load_values();

    ui_manager_event(ev);
    wl_connector_draw();
}

void render(uint32_t time, uint32_t index, bm_rgba_t* bm)
{
    ui_manager_render(0, bm);
}

void destroy()
{
    ui_destroy();
}

int main(int argc, char* argv[])
{
    zc_log_use_colors(isatty(STDERR_FILENO));
    zc_log_level_info();
    zc_time(NULL);

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
	"  -f, --frame=[width]x[height]          Initial window dimension\n"
	"  -m, --margin=[size]                   Margin\n"
	"  -r, --resources=[resources folder]    Resources dir for current session\n"
	"\n";

    const struct option long_options[] =
	{
	    {"help", no_argument, NULL, 'h'},
	    {"verbose", no_argument, NULL, 'v'},
	    {"frame", optional_argument, NULL, 'f'},
	    {"margin", optional_argument, NULL, 'm'},
	    {"resources", optional_argument, 0, 'r'}};

    char* res_par = NULL;
    char* frm_par = NULL;
    char* mrg_par = NULL;

    int verbose      = 0;
    int option       = 0;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "vhr:f:m:", long_options, &option_index)) != -1)
    {
	switch (option)
	{
	    case '?': printf("parsing option %c value: %s\n", option, optarg); break;
	    case 'r': res_par = cstr_new_cstring(optarg); break; // REL 0
	    case 'f': frm_par = cstr_new_cstring(optarg); break; // REL 1
	    case 'm': mrg_par = cstr_new_cstring(optarg); break; // REL 2
	    case 'v': verbose = 1; break;
	    default: fprintf(stderr, "%s", usage); return EXIT_FAILURE;
	}
    }

    srand((unsigned int) time(NULL));

    char cwd[PATH_MAX] = {"~"};
    if (getcwd(cwd, sizeof(cwd)) == NULL) printf("Cannot get working directory\n");

    char* wrk_path = path_new_normalize(cwd, NULL); // REL 3

    char* res_path     = NULL;
    char* res_path_loc = res_par ? path_new_normalize(res_par, wrk_path) : path_new_normalize("~/.config/wcp", getenv("HOME")); // REL 4
    char* res_path_glo = cstr_new_cstring(PKG_DATADIR);                                                                         // REL 5

    DIR* dir = opendir(res_path_loc);
    if (dir)
    {
	res_path = res_path_loc;
	closedir(dir);
    }
    else res_path = res_path_glo;

    char* css_path  = path_new_append(res_path, "html/main.css");  // REL 6
    char* html_path = path_new_append(res_path, "html/main.html"); // REL 7
    char* scr_path  = path_new_append(res_path, "script");         // REL 8

    // print path info to console

    printf("working path  : %s\n", wrk_path);
    printf("resource path : %s\n", res_path);
    printf("css path      : %s\n", css_path);
    printf("html path     : %s\n", html_path);
    printf("script path   : %s\n", scr_path);
    printf("\n");

    if (verbose) zc_log_inc_verbosity();

    // init config

    config_init(); // DESTROY 0
    config_set("res_path", res_path);

    // init non-configurable defaults

    config_set("wrk_path", wrk_path);
    config_set("css_path", css_path);
    config_set("html_path", html_path);
    config_set("scr_path", scr_path);

    int width  = 300;
    int height = 300;
    int margin = 0;

    if (frm_par != NULL)
    {
	width      = atoi(frm_par);
	char* next = strstr(frm_par, "x");
	height     = atoi(next + 1);
    }

    if (mrg_par != NULL)
    {
	margin = atoi(mrg_par);
    }

    wl_connector_init(width, height, margin, init, update, render, destroy);

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

#ifdef DEBUG
    mem_stats();
#endif
}
