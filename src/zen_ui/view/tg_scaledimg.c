/*
  CSS texture generator
  Generates texture based on css style
 */

#ifndef texgen_scaledimg_h
#define texgen_scaledimg_h

#include "view.c"
#include "zc_bm_rgba.c"

typedef struct _tg_scaledimg_t
{
    int        w;
    int        h;
    bm_rgba_t* bitmap;
} tg_scaledimg_t;

void tg_scaledimg_add(view_t* view, int w, int h);
void tg_scaledimg_gen(view_t* view);
void tg_scaledimg_set_content_size(view_t* view, int w, int h);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_log.c"

void tg_scaledimg_gen(view_t* view)
{
    tg_scaledimg_t* gen = view->tex_gen_data;
    bm_rgba_t*      bm  = view->texture.bitmap;

    if (bm == NULL ||
	bm->w != (int) gen->w ||
	bm->h != (int) gen->h)
    {
	bm = bm_rgba_new(gen->w, gen->h); // REL 0

	gfx_rect(bm, 0, 0, bm->w, bm->h, 0x00000000, 0);

	view_set_texture_bmp(view, bm);
	REL(bm);

	view->texture.changed = 1;
    }

    view->texture.state = TS_READY;
}

void tg_scaledimg_set_content_size(view_t* view, int w, int h)
{
    tg_scaledimg_t* gen = view->tex_gen_data;

    gen->w = w;
    gen->h = h;
}

void tg_scaledimg_add(view_t* view, int w, int h)
{
    assert(view->tex_gen == NULL);

    tg_scaledimg_t* gen = CAL(sizeof(tg_scaledimg_t), NULL, NULL);
    gen->w              = w;
    gen->h              = h;

    view->tex_gen_data = gen;
    view->tex_gen      = tg_scaledimg_gen;
    view->exclude      = 0;
}

#endif
