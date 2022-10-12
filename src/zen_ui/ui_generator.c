/*
  UI Generator Module for Zen Multimedia Desktop System
  Monitors views for changes, renders their content in the background, updates compositor state based on view state.
  All views have to be added to ui_generator, hierarchy is not handled.

 */

#ifndef ui_generator_h
#define ui_generator_h

#include "view.c"

int      ui_generator_init(int, int);
void     ui_generator_destroy();
void     ui_generator_render(uint32_t, bm_rgba_t* bm);
void     ui_generator_use(vec_t* views);
void     ui_generator_resize(int width, int height);
uint32_t ui_generate_create_texture();
void     ui_generator_rerender();

#endif

#if __INCLUDE_LEVEL__ == 0

#include "view.c"
#include "zc_channel.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_map.c"
#include "zc_vector.c"
#include <pthread.h>
#include <unistd.h>

int ui_generator_workloop(void* mypointer);

struct _ui_generator_t
{
    vec_t*     views;
    pthread_t* thread;
    ch_t*      channel;
    int        width;
    int        height;
    char       grow;
    int        wpwr;
    int        hpwr;
} uig = {0};

int nxt_pwr(int size)
{
    int val = 512;
    while (val < size) val *= 2;
    return val;
}

int ui_generator_init(int width, int height)
{
    uig.width  = width;
    uig.height = height;

    uig.views   = VNEW();     // REL 0
    uig.channel = ch_new(50); // REL 0

    int wp = nxt_pwr(width);
    int hp = nxt_pwr(height);

    uig.wpwr = wp;
    uig.hpwr = hp;

    return (uig.thread != NULL);
}

void ui_generator_destroy()
{
    REL(uig.views);
    REL(uig.channel);
}

void ui_generator_use(vec_t* views)
{
    vec_reset(uig.views);
    vec_add_in_vector(uig.views, views);
}

void ui_generator_resize(int width, int height)
{
    uig.width  = width;
    uig.height = height;

    int wp = nxt_pwr(width);
    int hp = nxt_pwr(height);

    // resize framebuffers if screen size changes
    if (wp != uig.wpwr || hp != uig.hpwr)
    {
	uig.wpwr = wp;
	uig.hpwr = hp;
    }
}

void ui_generator_render(uint32_t time, bm_rgba_t* bm)
{
    int reset_texmap = 0;

    for (int i = 0; i < uig.views->length; i++)
    {
	view_t* view = uig.views->data[i];

	if (view->texture.type == TT_MANAGED && view->texture.state == TS_BLANK)
	{
	    /* if (view->texture.rentype == RT_BACKGROUND) */
	    /* { */
	    /*   // printf("SENDING TEXTURE %s %f %f\n", view->style.background_image, view->frame.local.w, view->frame.local.h); */
	    /*   if (ch_send(uig.channel, view)) view->texture.state = TS_PENDING; */
	    /* } */
	    /* else */
	    /* { */
	    view_gen_texture(view);
	    /* } */
	}

	if (view->texture.changed)
	{
	    /* reset_texmap |= ui_compositor_upd_bmp(i, view->frame.global, view->style.shadow_blur, view->texture.bitmap); */

	    view->texture.changed = 0;
	}

	if (view->frame.dim_changed)
	{
	    /* ui_compositor_upd_dim(i, view->frame.global, view->style.shadow_blur); */

	    view->frame.dim_changed = 0;
	}

	if (view->frame.pos_changed)
	{
	    /* ui_compositor_upd_pos(i, view->frame.global, view->style.shadow_blur); */

	    view->frame.pos_changed = 0;
	}

	if (view->frame.reg_changed)
	{
	    /* ui_compositor_upd_region(i, view->frame.global, view->frame.region); */

	    view->frame.reg_changed = 0;
	}

	if (view->texture.alpha_changed)
	{
	    /* ui_compositor_upd_alpha(i, view->texture.alpha); */

	    view->texture.alpha_changed = 0;
	}

	// draw view into bitmap

	if (view->texture.bitmap)
	{
	    gfx_blend_bitmap(bm, view->texture.bitmap, view->frame.global.x, view->frame.global.y);
	}
    }

    /* ui_compositor_render(time, uig.width, uig.height, uig.wpwr, uig.hpwr); */

    /* if (reset_texmap || uig.grow) */
    /* { */
    /* 	printf("texture is full in generator render loop, forcing reset, grow : %i\n", uig.grow); */
    /* 	if (uig.grow) */
    /* 	{ */
    /* 	    uig.texmapsize *= 2; */
    /* 	    ui_generator_resize_texmap(uig.texmapsize); */
    /* 	} */
    /* 	ui_generator_resend_views(); */
    /* } */
}

int ui_generator_workloop(void* mypointer)
{
    view_t* view;

    while (1)
    {
	while ((view = ch_recv(uig.channel)))
	{
	    view_gen_texture(view);
	}
	/* SDL_Delay(16); */
    }
}

void ui_generator_rerender()
{
    for (int i = 0; i < uig.views->length; i++)
    {
	view_t* view = uig.views->data[i];
	if (view->texture.type == TT_MANAGED) view->texture.state = TS_BLANK;
    }
}

#endif
