
#ifndef texgen_knob_h
#define texgen_knob_h

#include "view.c"

typedef struct _tg_knob_t
{
    float      angle;
    bm_rgba_t* back;
    bm_rgba_t* fore;
} tg_knob_t;

void tg_knob_add(view_t* view);
void tg_knob_set_angle(view_t* view, float angle);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_draw.c"

void tg_knob_gen(view_t* view)
{
    tg_knob_t* tg = view->tex_gen_data;

    if (view->frame.local.w > 0 && view->frame.local.h > 0)
    {
	if (view->texture.bitmap == NULL && view->frame.local.w > 0 && view->frame.local.h > 0)
	{
	    bm_rgba_t* bmp = bm_rgba_new(view->frame.local.w, view->frame.local.h); // REL 0
	    tg->back       = bm_rgba_new(view->frame.local.w, view->frame.local.h); // REL 1
	    tg->fore       = bm_rgba_new(view->frame.local.w, view->frame.local.h); // REL 2

	    uint32_t basecol   = 0x454545FF;
	    uint32_t outercol  = 0x343434FF;
	    uint32_t centercol = 0x676767FF;
	    uint32_t shadowcol = 0xABABAB0A;

	    /* gfx_arc_grad(tg->back, */
	    /*              (view->frame.local.w - 1.0) / 2.0, */
	    /*              (view->frame.local.h - 1.0) / 2.0, */
	    /*              (view->frame.local.w / 2.0) - 3.0, */
	    /*              (view->frame.local.w / 2.0), */
	    /*              0, */
	    /*              3.14 * 2, */
	    /*              0x00000044, */
	    /*              0); */

	    gfx_arc_grad(tg->back, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 0, (view->frame.local.w / 2.0) - 5.0, 0, 3.14 * 2, basecol, basecol);

	    gfx_arc_grad(tg->back, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 27, 35, 0, 3.14 * 2, outercol, outercol);

	    gfx_arc_grad(tg->back, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, (view->frame.local.w / 2.0) - 5.0, (view->frame.local.w / 2.0) - 2.0, 0, 3.14 * 2, shadowcol, 0x00000000);

	    gfx_arc_grad(tg->fore, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 27.0, 31.0, 0, 3.14 * 2, shadowcol, 0);

	    gfx_arc_grad(tg->fore, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 0, 28.0, 0, 3.14 * 2, centercol, centercol);

	    view_set_texture_bmp(view, bmp);
	    REL(bmp); // REL 0
	}

	if (tg->angle < 0) tg->angle += 6.28;

	gfx_insert(view->texture.bitmap, tg->back, 0, 0);

	if (tg->angle > 3.14 * 3 / 2)
	{
	    gfx_arc_grad(view->texture.bitmap, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 27.0, 35.0, 3.14 * 3 / 2, tg->angle, 0x999999FF, 0x999999FF);
	}
	else
	{
	    gfx_arc_grad(view->texture.bitmap, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 27.0, 35.0, 3.14 * 3 / 2, 6.28, 0x999999FF, 0x999999FF);

	    gfx_arc_grad(view->texture.bitmap, (view->frame.local.w - 1.0) / 2.0, (view->frame.local.h - 1.0) / 2.0, 27.0, 35.0, 0, tg->angle, 0x999999FF, 0x999999FF);
	}

	gfx_blend_rgba(view->texture.bitmap, 0, 0, tg->fore);
	view->texture.changed = 1;
	view->texture.state   = TS_READY;
    }
}

void tg_knob_del(void* p)
{
    tg_knob_t* tg = p;
    if (tg->back) REL(tg->back);
    if (tg->fore) REL(tg->fore);
}

void tg_knob_desc(void* p, int level)
{
    printf("tg_knob");
}

void tg_knob_add(view_t* view)
{
    assert(view->tex_gen == NULL);

    tg_knob_t* tg = CAL(sizeof(tg_knob_t), tg_knob_del, tg_knob_desc);
    tg->angle     = 3 * 3.14 / 2;

    view->tex_gen_data = tg;
    view->tex_gen      = tg_knob_gen;
    view->exclude      = 0;

    view->needs_touch   = 1;
    view->blocks_touch  = 1;
    view->blocks_scroll = 1;
}

void tg_knob_set_angle(view_t* view, float angle)
{
    tg_knob_t* tg = view->tex_gen_data;

    tg->angle           = angle;
    view->texture.state = TS_BLANK; // force rerender
}

#endif
