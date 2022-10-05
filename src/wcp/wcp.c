#include "wl_connector.c"
#include "zc_log.c"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    printf("startup...\n");
    zc_log_use_colors(isatty(STDERR_FILENO));
    zc_log_level_debug();

    wl_connector_init(300, 200);
}
