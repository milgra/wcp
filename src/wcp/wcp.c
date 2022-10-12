#include "config.c"
#include "ui.c"
#include "ui_manager.c"
#include "wl_connector.c"
#include "zc_bm_rgba.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_path.c"
#include "zc_time.c"
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
	   "by Milan Toth(www.milgra.com)\n"
	   "If you like this app try :\n"
	   "- Sway Oveview ( github.com/milgra/sov )\n"
	   "- Visual Music Player (github.com/milgra/vmp)\n"
	   "- Multimedia File Manager (github.com/milgra/wcp)\n"
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
	"  -r, --resources= [resources folder]    Resources dir for current session\n"
	"  -f, --frame= [widthxheight]            Initial window dimension\n"
	"\n";

    const struct option long_options[] =
	{
	    {"help", no_argument, NULL, 'h'},
	    {"verbose", no_argument, NULL, 'v'},
	    {"resources", optional_argument, 0, 'r'},
	    {"config", optional_argument, 0, 'c'}};

    char* cfg_par = NULL;
    char* res_par = NULL;
    char* frm_par = NULL;

    int verbose      = 0;
    int option       = 0;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "vhr:c:f:p:o", long_options, &option_index)) != -1)
    {
	switch (option)
	{
	    case '?': printf("parsing option %c value: %s\n", option, optarg); break;
	    case 'c': cfg_par = cstr_new_cstring(optarg); break; // REL 0
	    case 'r': res_par = cstr_new_cstring(optarg); break; // REL 1
	    case 'f': frm_par = cstr_new_cstring(optarg); break; // REL 4
	    case 'v': verbose = 1; break;
	    default: fprintf(stderr, "%s", usage); return EXIT_FAILURE;
	}
    }

    srand((unsigned int) time(NULL));

    char cwd[PATH_MAX] = {"~"};
    getcwd(cwd, sizeof(cwd));

    char* top_path    = path_new_normalize(cwd, NULL);                                                                         // REL 5
    char* res_path    = res_par ? path_new_normalize(res_par, top_path) : cstr_new_cstring(PKG_DATADIR);                       // REL 7
    char* cfgdir_path = cfg_par ? path_new_normalize(cfg_par, top_path) : path_new_normalize("~/.config/wcp", getenv("HOME")); // REL 8
    char* css_path    = path_new_append(res_path, "html/main.css");                                                            // REL 9
    char* html_path   = path_new_append(res_path, "html/main.html");                                                           // REL 10
    char* scr_path    = path_new_append(res_path, "script");                                                                   // REL 10
    char* cfg_path    = path_new_append(cfgdir_path, "config.kvl");                                                            // REL 12
    char* per_path    = path_new_append(cfgdir_path, "state.kvl");                                                             // REL 13

    // print path info to console

    printf("top path      : %s\n", top_path);
    printf("resource path : %s\n", res_path);
    printf("config path   : %s\n", cfg_path);
    printf("state path    : %s\n", per_path);
    printf("css path      : %s\n", css_path);
    printf("html path     : %s\n", html_path);
    printf("script path     : %s\n", scr_path);
    printf("\n");

    if (verbose) zc_log_inc_verbosity();

    // init config

    config_init(); // DESTROY 0
    config_set("res_path", res_path);

    // init non-configurable defaults

    config_set("top_path", top_path);
    config_set("cfg_path", cfg_path);
    config_set("per_path", per_path);
    config_set("css_path", css_path);
    config_set("html_path", html_path);
    config_set("scr_path", scr_path);

    int width  = 300;
    int height = 300;
    if (frm_par != NULL)
    {
	width      = atoi(frm_par);
	char* next = strstr(frm_par, "x");
	height     = atoi(next + 1);
    }

    wl_connector_init(width, height, init, update, render, destroy);

    /* show in wayland buffer */

    config_destroy(); // DESTROY 0

    // cleanup

    if (cfg_par) REL(cfg_par); // REL 0
    if (res_par) REL(res_par); // REL 1
    if (frm_par) REL(frm_par); // REL 4

    REL(top_path);    // REL 5
    REL(res_path);    // REL 7
    REL(cfgdir_path); // REL 8
    REL(css_path);    // REL 9
    REL(html_path);   // REL 10
    REL(scr_path);    // REL 10
    REL(cfg_path);    // REL 12
    REL(per_path);    // REL 13

#ifdef DEBUG
    /* mem_stats(); */
#endif
}
