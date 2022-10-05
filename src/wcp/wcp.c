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

void render(uint32_t time, bm_rgba_t* bm)
{
    printf("RENDER\n");

    gfx_rect(bm, 100, 100, 40, 40, 0x00ff00ff, 0);

    ui_manager_render(0, bm);
}

int main(int argc, char* argv[])
{
    zc_log_use_colors(isatty(STDERR_FILENO));
    zc_log_level_info();
    zc_time(NULL);

    printf("Visual Music Player v" WCP_VERSION " by Milan Toth ( www.milgra.com )\n");
    printf("If you like this app try Multimedia File Manager (github.com/milgra/wcp) or Sway Oveview ( github.com/milgra/sov )\n");
    printf("Or my games : Cortex ( github.com/milgra/cortex ), Termite (github.com/milgra/termite) or Brawl (github.com/milgra/brawl)\n\n");

    const char* usage =
	"Usage: wcp [options]\n"
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

    srand((unsigned int) time(NULL));

    char cwd[PATH_MAX] = {"~"};
    getcwd(cwd, sizeof(cwd));

    char* top_path = path_new_normalize(cwd, NULL); // REL 5
    /* char* sdl_base = SDL_GetBasePath(); */
    /* char* wrk_path = path_new_normalize(sdl_base, NULL); // REL 6 */
    /* SDL_free(sdl_base); */
    char* wrk_path    = top_path;
    char* res_path    = res_par ? path_new_normalize(res_par, wrk_path) : cstr_new_cstring(PKG_DATADIR);                       // REL 7
    char* cfgdir_path = cfg_par ? path_new_normalize(cfg_par, wrk_path) : path_new_normalize("~/.config/wcp", getenv("HOME")); // REL 8
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

    // init non-configurable defaults

    config_set("top_path", top_path);
    config_set("wrk_path", wrk_path);
    config_set("cfg_path", cfg_path);
    config_set("per_path", per_path);
    config_set("css_path", css_path);
    config_set("html_path", html_path);

    if (rec_path) config_set("rec_path", rec_path);
    if (rep_path) config_set("rep_path", rep_path);

    zc_time(NULL);
    ui_init(300, 200); // DESTROY 3
    zc_time("ui init");

    /* ui_manager_event(ev); */

    wl_connector_init(300, 200, render);

    /* show in wayland buffer */

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
}
