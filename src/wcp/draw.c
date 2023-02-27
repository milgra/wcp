#include "config.c"
#include "ku_bitmap_ext.c"
#include "ku_renderer_soft.c"
#include "ku_window.c"
#include "mt_path.c"
#include "mt_string.c"
#include "ui.c"
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int   width;
int   height;
int   margin;
char* anchor;
float scale;

int main(int argc, char* argv[])
{
    const struct option long_options[] = {
	{"anchor", optional_argument, NULL, 'a'},
	{"frame", optional_argument, NULL, 'f'},
	{"margin", optional_argument, NULL, 'm'},
	{"resources", optional_argument, 0, 'r'},
	{"target", optional_argument, 0, 't'}};

    char* res_par = NULL;
    char* frm_par = NULL;
    char* mrg_par = NULL;
    char* anc_par = NULL;
    char* tgt_par = NULL;

    int option       = 0;
    int option_index = 0;

    while ((option = getopt_long(argc, argv, "vhr:f:m:a:t:", long_options, &option_index)) != -1)
    {
	switch (option)
	{
	    case '?': printf("parsing option %c value: %s\n", option, optarg); break;
	    case 'a': anc_par = mt_string_new_cstring(optarg); break; // REL 0
	    case 'r': res_par = mt_string_new_cstring(optarg); break; // REL 0
	    case 'f': frm_par = mt_string_new_cstring(optarg); break; // REL 1
	    case 'm': mrg_par = mt_string_new_cstring(optarg); break; // REL 2
	    case 't': tgt_par = mt_string_new_cstring(optarg); break; // REL 2
	    default: return EXIT_FAILURE;
	}
    }

    srand((unsigned int) time(NULL));

    char* res_path     = NULL;
    char* res_path_loc = res_par ? mt_path_new_normalize(res_par) : mt_path_new_normalize("~/.config/wcp"); // REL 4
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

    printf("resource path : %s\n", res_path);
    printf("css path      : %s\n", css_path);
    printf("html path     : %s\n", html_path);
    printf("image path    : %s\n", img_path);
    printf("script path   : %s\n", scr_path);
    printf("target path   : %s\n", tgt_par);
    printf("\n");

    config_init(); // DESTROY 0
    config_set("res_path", res_path);

    config_set("css_path", css_path);
    config_set("html_path", html_path);
    config_set("scr_path", scr_path);
    config_set("img_path", img_path);

    width  = 300;
    height = 300;
    margin = 0;
    scale  = 1.0;

    if (anc_par) anchor = anc_par;
    else anchor = "";

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

    ku_window_t* kuwindow = ku_window_create(width * scale, height * scale, scale);

    ui_init(width, height, scale, kuwindow, NULL);
    ku_window_update(kuwindow, 0);

    ku_bitmap_t* bitmap = ku_bitmap_new(width, height);

    ku_renderer_software_render(kuwindow->views, bitmap, kuwindow->root->frame.local);

    bm_write(bitmap, tgt_par);

    REL(bitmap);
    
    // cleanup

    if (res_par) REL(res_par); // REL 0
    if (frm_par) REL(frm_par); // REL 1
    if (mrg_par) REL(mrg_par); // REL 1
    if (tgt_par) REL(tgt_par); // REL 1

    REL(res_path_loc); // REL 4
    REL(res_path_glo); // REL 5
    REL(css_path);     // REL 6
    REL(html_path);    // REL 7
    REL(scr_path);     // REL 8
    REL(img_path);
}
