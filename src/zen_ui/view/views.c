#ifndef views_h
#define views_h

#include "zc_map.c"

typedef struct _views_t
{
    map_t* names;
    int    arrange;
} views_t;

extern views_t views;

void views_init();
void views_destroy();
void views_describe();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "view.c"

views_t views = {0};

void views_init()
{
    views.names   = MNEW();
    views.arrange = 0;
}

void views_destroy()
{
#ifdef DEBUG
    printf("***VIEW STATS***\n");
    printf("UNRELEASED VIEWS : %i\n", views.names->count);
    if (views.names->count > 0) map_describe(views.names, 0);
#endif

    REL(views.names);
    views.names = NULL;
}

#endif
