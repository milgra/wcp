#include "wl_connector.c"
#include <stdio.h>

int main(int argc, char* argv[])
{
    printf("startup...\n");

    wl_connector_init(300, 200);
}
