#include "config.c"
#include "evrecorder.c"
#include "filemanager.c"
#include "ui.c"
#include "ui_compositor.c"
#include "ui_manager.c"
#include "wm_connector.c"
#include "zc_cstring.c"
#include "zc_log.c"
#include "zc_map.c"
#include "zc_path.c"
#include "zc_time.c"
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct
{
    char replay;
    char record;
    int  frames;
} ocp = {0};

void init(int width, int height)
{
    zc_time(NULL);
    ui_init(width, height); // DESTROY 3
    zc_time("ui init");

    if (ocp.record)
    {
	evrec_init_recorder(config_get("rec_path")); // DESTROY 4
    }

    if (ocp.replay)
    {
	evrec_init_player(config_get("rep_path")); // DESTROY 5
	ui_add_cursor();
    }
}

void post_render_init()
{
    if (ocp.frames < 3) ocp.frames += 1;
    else if (ocp.frames == 3)
    {
	ocp.frames = 4;

	ui_post_render_init();
    }
}

void update(ev_t ev)
{
    if (ev.type == EV_TIME)
    {
	/* check init */

	if (ocp.frames < 4) post_render_init();

	if (ocp.replay)
	{
	    // get recorded events
	    ev_t* recev = NULL;
	    while ((recev = evrec_replay(ev.time)) != NULL)
	    {
		ui_manager_event(*recev);
		ui_update_cursor((r2_t){recev->x, recev->y, 10, 10});

		if (recev->type == EV_KDOWN && recev->keycode == SDLK_PRINTSCREEN) ui_screenshot(ev.time);
	    }
	}
    }
    else
    {
	if (ocp.record)
	{
	    evrec_record(ev);
	    if (ev.type == EV_KDOWN && ev.keycode == SDLK_PRINTSCREEN) ui_screenshot(ev.time);
	}
    }

    // in case of replay only send time events
    if (!ocp.replay || ev.type == EV_TIME) ui_manager_event(ev);
}

void render(uint32_t time)
{
    ui_manager_render(time);
}

void destroy()
{
    if (ocp.replay) evrec_destroy(); // DESTROY 5
    if (ocp.record) evrec_destroy(); // DESTROY 4

    ui_destroy(); // DESTROY 3
}

int main(int argc, char* argv[])
{
    zc_log_use_colors(isatty(STDERR_FILENO));
    zc_log_level_info();
    zc_time(NULL);

    printf("Visual Music Player v" OCP_VERSION " by Milan Toth ( www.milgra.com )\n");
    printf("If you like this app try Multimedia File Manager (github.com/milgra/ocp) or Sway Oveview ( github.com/milgra/sov )\n");
    printf("Or my games : Cortex ( github.com/milgra/cortex ), Termite (github.com/milgra/termite) or Brawl (github.com/milgra/brawl)\n\n");

    const char* usage =
	"Usage: ocp [options]\n"
	"\n"
	"  -h, --help                            Show help message and quit.\n"
	"  -v, --verbose                         Increase verbosity of messages, defaults to errors and warnings only.\n"
	"  -r, --resources= [resources folder]    Resources dir for current session\n"
	"  -s, --record= [recorder file]          Record session to file\n"
	"  -p, --replay= [recorder file]          Replay session from file\n"
	"  -f, --frame= [widthxheight]            Initial window dimension\n"
	"  -l, --location= [horxver]             Initial window position\n"
	"\n";

    const struct option long_options[] =
	{
	    {"help", no_argument, NULL, 'h'},
	    {"verbose", no_argument, NULL, 'v'},
	    {"resources", optional_argument, 0, 'r'},
	    {"record", optional_argument, 0, 's'},
	    {"replay", optional_argument, 0, 'p'},
	    {"config", optional_argument, 0, 'c'},
	    {"location", optional_argument, 0, 'l'}};

    char* cfg_par = NULL;
    char* res_par = NULL;
    char* rec_par = NULL;
    char* rep_par = NULL;
    char* frm_par = NULL;
    char* loc_par = NULL;

    int verbose      = 0;
    int option       = 0;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "vhr:s:p:c:f:p:l:o", long_options, &option_index)) != -1)
    {
	switch (option)
	{
	    case '?': printf("parsing option %c value: %s\n", option, optarg); break;
	    case 'c': cfg_par = cstr_new_cstring(optarg); break; // REL 0
	    case 'l': loc_par = cstr_new_cstring(optarg); break; // REL 1
	    case 'r': res_par = cstr_new_cstring(optarg); break; // REL 1
	    case 's': rec_par = cstr_new_cstring(optarg); break; // REL 2
	    case 'p': rep_par = cstr_new_cstring(optarg); break; // REL 3
	    case 'f': frm_par = cstr_new_cstring(optarg); break; // REL 4
	    case 'v': verbose = 1; break;
	    default: fprintf(stderr, "%s", usage); return EXIT_FAILURE;
	}
    }

    if (rec_par) ocp.record = 1;
    if (rep_par) ocp.replay = 1;

    srand((unsigned int) time(NULL));

    char cwd[PATH_MAX] = {"~"};
    getcwd(cwd, sizeof(cwd));

    char* top_path = path_new_normalize(cwd, NULL); // REL 5
    char* sdl_base = SDL_GetBasePath();
    char* wrk_path = path_new_normalize(sdl_base, NULL); // REL 6
    SDL_free(sdl_base);
    char* res_path    = res_par ? path_new_normalize(res_par, wrk_path) : cstr_new_cstring(PKG_DATADIR);                       // REL 7
    char* cfgdir_path = cfg_par ? path_new_normalize(cfg_par, wrk_path) : path_new_normalize("~/.config/ocp", getenv("HOME")); // REL 8
    char* css_path    = path_new_append(res_path, "html/main.css");                                                            // REL 9
    char* html_path   = path_new_append(res_path, "html/main.html");                                                           // REL 10
    char* cfg_path    = path_new_append(cfgdir_path, "config.kvl");                                                            // REL 12
    char* per_path    = path_new_append(cfgdir_path, "state.kvl");                                                             // REL 13
    char* rec_path    = rec_par ? path_new_normalize(rec_par, wrk_path) : NULL;                                                // REL 14
    char* rep_path    = rep_par ? path_new_normalize(rep_par, wrk_path) : NULL;                                                // REL 15

    // print path info to console

    printf("top path      : %s\n", top_path);
    printf("working path  : %s\n", wrk_path);
    printf("resource path : %s\n", res_path);
    printf("config path   : %s\n", cfg_path);
    printf("state path    : %s\n", per_path);
    printf("css path      : %s\n", css_path);
    printf("html path     : %s\n", html_path);
    printf("record path   : %s\n", rec_path);
    printf("replay path   : %s\n", rep_path);
    printf("\n");

    if (verbose) zc_log_inc_verbosity();

    // init config

    config_init(); // DESTROY 0

    config_set("res_path", res_path);

    // read config, it overwrites defaults if exists

    config_read(cfg_path);

    // init non-configurable defaults

    config_set("top_path", top_path);
    config_set("wrk_path", wrk_path);
    config_set("cfg_path", cfg_path);
    config_set("per_path", per_path);
    config_set("css_path", css_path);
    config_set("html_path", html_path);

    if (rec_path) config_set("rec_path", rec_path);
    if (rep_path) config_set("rep_path", rep_path);

    zc_time("config parsing");

    wm_loop(init, update, render, destroy, frm_par, loc_par);

    config_destroy(); // DESTROY 0

    // cleanup

    if (cfg_par) REL(cfg_par); // REL 0
    if (res_par) REL(res_par); // REL 1
    if (rec_par) REL(rec_par); // REL 2
    if (rep_par) REL(rep_par); // REL 3
    if (frm_par) REL(frm_par); // REL 4

    REL(top_path);    // REL 5
    REL(wrk_path);    // REL 6
    REL(res_path);    // REL 7
    REL(cfgdir_path); // REL 8
    REL(css_path);    // REL 9
    REL(html_path);   // REL 10
    REL(cfg_path);    // REL 12
    REL(per_path);    // REL 13

    if (rec_path) REL(rec_path); // REL 14
    if (rep_path) REL(rep_path); // REL 15

#ifdef DEBUG
	/* mem_stats(); */
#endif

    return 0;
}
