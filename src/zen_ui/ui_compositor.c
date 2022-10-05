/*
  UI Compositor Module for Zen Multimedia Desktop System
  Stores incoming rects
  Stores incoming textures
  Links rects with textures
  Uploads and renders rects on demand

  ui_compositor -> gl_connector -> GPU

 */

#ifndef ui_compositor_h
#define ui_compositor_h

#include "zc_bm_rgba.c"
#include "zc_util2.c"
#include "zc_vec2.c"

void ui_compositor_init();
void ui_compositor_destroy();

void ui_compositor_reset_texmap(int size);
void ui_compositor_new_texture(int page, int width, int height);
void ui_compositor_rel_texture(int page);
void ui_compositor_resize_texture(int page, int width, int height);

void ui_compositor_rewind();
void ui_compositor_add(
    char* id,
    char  masked,
    char  unmask,
    r2_t  frame,
    float border, // view border
    float alpha,
    int   page, // texture pagev
    int   full, // needs full texture
    int   ext,  // external texture
    int   tex_w,
    int   tex_h); // texture id
void ui_compositor_upd_pos(int index, r2_t frame, float border);
void ui_compositor_upd_dim(int index, r2_t frame, float border);
char ui_compositor_upd_bmp(int index, r2_t frame, float border, bm_rgba_t* bm);
void ui_compositor_upd_alpha(int index, float alpha);
void ui_compositor_upd_region(int index, r2_t frame, r2_t region);

void ui_compositor_render(uint32_t time, int width, int height, int wpwr, int hpwr);
void ui_compositor_render_to_bmp(bm_rgba_t* bitmap);

#endif

#if __INCLUDE_LEVEL__ == 0

#define UI_STAT_DELAY 10.0

#include "ui_texmap.c"
#include "zc_cstring.c"
#include "zc_log.c"
#include "zc_map.c"
#include "zc_mat4.c"
#include "zc_vector.c"

typedef struct _crect_t
{
    char* id;
    float data[36];
    char  masked;
    char  unmask;
    r2_t  frame;
} crect_t;

void crect_del(void* rect);
void crect_desc(void* p, int level);
void crect_set_id(crect_t* rect, char* id);
void crect_set_masked(crect_t* r, char masked);
void crect_set_unmask(crect_t* r, char unmask);
void crect_set_page(crect_t* rect, uint32_t page);
void crect_set_alpha(crect_t* r, float alpha);
void crect_set_frame(crect_t* rect, r2_t uirect);
void crect_set_texture(crect_t* rect, float tlx, float tly, float brx, float bry);

struct uic_t
{
    /* fb_t* fb;      // float buffer */
    tm_t* tm;      // texture map
    int   upd_geo; // update geometry

    vec_t* cache;
    int    cache_ind;

    uint32_t tex_bytes;
    uint32_t ver_bytes;
    uint32_t upd_stamp;
    uint32_t ren_frame;
} uic = {0};

void ui_compositor_init()
{
    // gl_init();

    /* uic.fb        = fb_new(); // REL 0 */
    uic.cache     = VNEW(); // REL 1
    uic.cache_ind = 0;
    uic.upd_geo   = 1;
}

void ui_compositor_destroy()
{
    /* REL(uic.fb); */
    REL(uic.cache);
    if (uic.tm) REL(uic.tm);

    // gl_destroy();
}

void ui_compositor_rewind()
{
    uic.cache_ind = 0;
}

void ui_compositor_reset_texmap(int size)
{
    if (uic.tm) REL(uic.tm);
    uic.tm = tm_new(size, size); // REL 2
}

void ui_compositor_new_texture(int page, int width, int height)
{
    // gl_new_texture(page, width, height);
}

void ui_compositor_rel_texture(int page)
{
    // gl_rel_texture(page);
}

void ui_compositor_resize_texture(int page, int width, int height)
{
    // gl_rel_texture(page);
    // gl_new_texture(page, width, height);
}

void ui_compositor_add(
    char* id,
    char  masked,
    char  unmask,
    r2_t  frame,
    float border, // view border
    float alpha,
    int   page, // texture page
    int   full, // needs full texture
    int   ext,  // external texture
    int   tex_w,
    int   tex_h)
{
    /* printf("ui_compositor_add %s %f %f %f %f masked %i\n", id, frame.x, frame.y, frame.w, frame.h, masked); */

    // fill up cache if needed
    if (uic.cache_ind + 1 > uic.cache->length)
    {
	crect_t* rect = CAL(sizeof(crect_t), crect_del, crect_desc); // REL 0
	VADD(uic.cache, rect);
	REL(rect); // REL 0
    }
    // get cached rect
    crect_t* rect = uic.cache->data[uic.cache_ind];

    // set id
    crect_set_id(rect, id);

    // set masked
    crect_set_masked(rect, masked);

    // set umask
    crect_set_unmask(rect, unmask);

    // set frame
    if (border > 0.0) frame = r2_expand(frame, border);
    crect_set_frame(rect, frame);

    // set page
    crect_set_page(rect, page);

    // set alpha
    crect_set_alpha(rect, alpha);

    // TEXTURE COORDS

    // by default view gets full texture
    crect_set_texture(rect, 0.0, 0.0, 0.0, 0.0);

    if (full)
    {
	crect_set_texture(rect, 0.0, 0.0, 1.0, 1.0);
    }
    else if (ext)
    {
	if (frame.w > 0 && frame.h > 0)
	{
	    crect_set_texture(rect, 0.0, 0.0, frame.w / (float) tex_w, frame.h / (float) tex_h);
	}
    }
    else
    {
	// prepare texmap
	tm_coords_t tc = tm_get(uic.tm, id);

	if (tc.w > 0 && tc.h > 0)
	{

	    if ((int) frame.w != tc.w || (int) frame.h != tc.h)
	    {
		// texture doesn't exist or size mismatch
		int success = tm_put(uic.tm, id, frame.w, frame.h);
		// TODO reset main texture, maybe all views?
		if (success < 0) printf("TEXTURE FULL, NEEDS RESET\n");

		// update tex coords
		tc = tm_get(uic.tm, id);
	    }

	    // set texture coords
	    crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);
	}
    }

    // increase cache index
    uic.cache_ind++;
    uic.upd_geo = 1;
}

void ui_compositor_upd_pos(int index, r2_t frame, float border)
{
    crect_t* rect  = uic.cache->data[index];
    r2_t     prevf = rect->frame;
    prevf.x        = frame.x - border;
    prevf.y        = frame.y - border;

    crect_set_frame(rect, prevf);

    uic.upd_geo = 1;
}

void ui_compositor_upd_dim(int index, r2_t frame, float border)
{
    crect_t* rect = uic.cache->data[index];

    // printf("ui_compositor_upd_dim %s w %i h %i\n", rect->id, bm->w, bm->h);

    if (border > 0.0) frame = r2_expand(frame, border);
    crect_set_frame(rect, frame);

    uic.upd_geo = 1;
}

// show only region of view, modify vertexes and texture coords

void ui_compositor_upd_region(int index, r2_t frame, r2_t region)
{
    crect_t* rect = uic.cache->data[index];

    // printf("update region %s %f %f %f %f\n", rect->id, region.x, region.y, region.x + region.w, region.y + region.h);

    r2_t of = frame;
    of.x += region.x;
    of.y += region.y;
    of.w = region.w;
    of.h = region.h;

    tm_coords_t tc = tm_get(uic.tm, rect->id);
    // tm_coords_t nc = tc;

    float tw = tc.rbx - tc.ltx; // texcoord x distance
    float th = tc.rby - tc.lty; // texcoord y distance

    // left top
    float fx = region.x;
    float fy = region.y;
    float rx = fx / frame.w;
    float ry = fy / frame.h;
    float cx = tc.ltx + tw * rx;
    float cy = tc.lty + th * ry;

    // right bottom
    fx         = region.x + region.w;
    fy         = region.y + region.h;
    rx         = fx / frame.w;
    ry         = fy / frame.h;
    float rbcx = tc.ltx + tw * rx;
    float rbcy = tc.lty + th * ry;

    crect_set_frame(rect, of);
    crect_set_texture(rect, cx, cy, rbcx, rbcy);

    /* printf("update region 1 %s %f %f %f %f\n", rect->id, tc.ltx, tc.lty, tc.rbx, tc.rby); */
    /* printf("update region 2 %s %f %f %f %f\n", rect->id, cx, cy, rbcx, rbcy); */

    uic.upd_geo = 1;
}

void ui_compositor_upd_alpha(int index, float alpha)
{
    crect_t* rect = uic.cache->data[index];
    crect_set_alpha(rect, alpha);
    uic.upd_geo = 1;
}

char ui_compositor_upd_bmp(int index, r2_t frame, float border, bm_rgba_t* bm)
{
    crect_t* rect = uic.cache->data[index];

    // printf("ui_compositor_upd_bmp %s w %i h %i\n", rect->id, bm->w, bm->h);

    frame.w = bm->w - 2 * border;
    frame.h = bm->h - 2 * border;

    if (border > 0.0) frame = r2_expand(frame, border);

    tm_coords_t tc = tm_get(uic.tm, rect->id);

    // TODO store texture size in texture id so older sizes can be reused on fullscreen switches?
    if (bm->w != tc.w || bm->h != tc.h)
    {
	// texture doesn't exist or size mismatch
	int success = tm_put(uic.tm, rect->id, frame.w, frame.h);
	// TODO reset main texture, maybe all views?
	if (success < 0)
	{
	    // printf("TEXTURE FULL, NEEDS RESET\n");
	    return 1;
	}

	// update tex coords
	tc = tm_get(uic.tm, rect->id);

	// set new texture coords
	crect_set_texture(rect, tc.ltx, tc.lty, tc.rbx, tc.rby);

	// resend is needed since a crect changed
	uic.upd_geo = 1;
    }

    // upload to GPU
    // gl_upload_to_texture(0, tc.x, tc.y, bm->w, bm->h, bm->data);

    uic.tex_bytes += bm->size;

    return 0;
}

void ui_compositor_render(uint32_t time, int width, int height, int tex_w, int tex_h)
{
    /* if (uic.upd_geo == 1) */
    /* { */
    /* 	/\* fb_reset(uic.fb); *\/ */
    /* 	for (int i = 0; i < uic.cache_ind; i++) */
    /* 	{ */
    /* 	    crect_t* rect = uic.cache->data[i]; */
    /* 	    //fb_add(uic.fb, rect->data, 36); */
    /* 	} */

    /* 	// gl_upload_vertexes(uic.fb); */

    /* 	uic.upd_geo = 0; */
    /* 	/\* uic.ver_bytes += uic.fb->pos * sizeof(GLfloat); *\/ */
    /* } */

    /* uic.ren_frame += 1; */

    /* // rendering region */
    /* glrect_t region   = {0, 0, width, height}; */
    /* glrect_t viewport = {0, 0, width, height}; */

    /* // reset main buffer */
    /* // gl_clear_framebuffer(TEX_CTX, 0.0, 0.0, 0.0, 1.0); */

    /* // reset mask */
    /* // gl_clear_framebuffer(1, 0.0, 0.0, 0.0, 1.0); */

    /* int last  = 0; */
    /* int index = 0; */

    /* for (int i = 0; i < uic.cache_ind; i++) */
    /* { */
    /* 	crect_t* rect = uic.cache->data[i]; */
    /* 	if (rect->masked) */
    /* 	{ */
    /* 	    // draw buffer so far into main buffer */
    /* 	    // gl_draw_vertexes_in_framebuffer(TEX_CTX, last * 6, index * 6, viewport, viewport, SH_TEXTURE, 0, tex_w, tex_h); */

    /* 	    region.x = rect->frame.x; */
    /* 	    region.y = rect->frame.y; */
    /* 	    region.w = rect->frame.w; */
    /* 	    region.h = rect->frame.h; */

    /* 	    // set last index */
    /* 	    last = index; */
    /* 	} */

    /* 	index += 1; */

    /* 	if (rect->unmask) */
    /* 	{ */
    /* 	    // draw buffer so far into main buffer with mask */
    /* 	    // gl_draw_vertexes_in_framebuffer(TEX_CTX, last * 6, index * 6, viewport, region, SH_TEXTURE, 0, tex_w, tex_h); */

    /* 	    // TODO jump back to previous mask if nested */

    /* 	    region.x = 0; */
    /* 	    region.y = 0; */
    /* 	    region.w = width; */
    /* 	    region.h = height; */

    /* 	    // set last index */
    /* 	    last = index; */
    /* 	} */
    /* } */

    /* // draw remaining buffer */
    /* if (last < index) */
    /* { */
    /* 	// gl_draw_vertexes_in_framebuffer(TEX_CTX, last * 6, index * 6, viewport, viewport, SH_TEXTURE, 0, tex_w, tex_h); */
    /* } */

    /* if (time > uic.upd_stamp) */
    /* { */
    /*   printf("UI TX %.2f Mb/s VX %.2f Mb/s FPS %.2f\n", */
    /*          uic.tex_bytes / UI_STAT_DELAY / (1024.0 * 1024.0), */
    /*          uic.ver_bytes / UI_STAT_DELAY / (1024.0 * 1024.0), */
    /*          uic.ren_frame / UI_STAT_DELAY); */
    /*   uic.upd_stamp = time + UI_STAT_DELAY * 1000.0; */
    /*   uic.tex_bytes = 0; */
    /*   uic.ver_bytes = 0; */
    /*   uic.ren_frame = 0; */
    /* } */
}

void ui_compositor_render_to_bmp(bm_rgba_t* bm)
{
    // proxy render_to_bmp to current backend which is opengl right now
    // gl_save_framebuffer(bm);
}

/* void ui_compositor_render() */
/* { */

/*   crect_t* rect; */
/*   int      last  = 0; */
/*   int      index = 0; */
/*   for (index = 0; index < uic.final_v->length; index++) */
/*   { */
/*     rect = uic.final_v->data[index]; */
/*     if (rect->shadow || rect->blur) */
/*     { */
/*       // render rects so far with simple texture renderer to offscreen buffer */
/*       gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_TEXTURE); */

/*       last = index; */

/*       if (rect->shadow) */
/*       { */
/*         // render current view with black color to an offscreen buffer */
/*         gl_clear_framebuffer(2, 0.0, 0.0, 0.0, 0.0); */
/*         gl_clear_framebuffer(3, 0.0, 0.0, 0.0, 0.0); */
/*         gl_draw_vertexes_in_framebuffer(2, index * 6, (index + 1) * 6, reg_full, reg_half, SH_COLOR); */
/*         // blur offscreen buffer for soft shadows */
/*         gl_draw_framebuffer_in_framebuffer(2, 3, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */
/*         // draw offscreen buffer on final buffer */
/*         gl_draw_framebuffer_in_framebuffer(3, 1, reg_half, reg_full, ((glrect_t){0}), SH_DRAW); */
/*       } */

/*       if (rect->blur) */
/*       { */
/*         // render current state with texture shader to an offscreen buffer */
/*         gl_clear_framebuffer(6, 0.0, 0.0, 0.0, 0.0); */
/*         gl_clear_framebuffer(5, 0.0, 0.0, 0.0, 0.0); */
/*         // shrink current framebuffer for blur */
/*         gl_draw_framebuffer_in_framebuffer(3, 6, reg_full, reg_half, ((glrect_t){0}), SH_DRAW); */

/*         // blur offscreen buffer for soft shadows */
/*         gl_draw_framebuffer_in_framebuffer(6, 5, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */
/*         gl_draw_framebuffer_in_framebuffer(5, 6, reg_half, reg_half, ((glrect_t){0}), SH_BLUR); */

/*         // draw blurred buffer on final buffer inside the view */
/*         gl_draw_framebuffer_in_framebuffer(6, 3, reg_half, reg_full, rect->region, SH_DRAW); */

/*         // skip drawing actual rect when blur */
/*         last++; */
/*         index++; */
/*       } */
/*     } */
/*   } */

/*   if (last < index) */
/*   { */
/*     // render remaining */
/*     gl_draw_vertexes_in_framebuffer(1, last * 6, index * 6, reg_full, reg_full, SH_DRAW); */
/*   } */

/*   // finally draw offscreen buffer to screen buffer */
/*   gl_draw_framebuffer_in_framebuffer(1, TEX_CTX, reg_full, reg_full, ((glrect_t){0}), SH_DRAW); */
/* } */

//
// Compositor Rect
//

void crect_del(void* pointer)
{
    crect_t* r = (crect_t*) pointer;
    REL(r->id);
}

void crect_set_id(crect_t* r, char* id)
{
    if (r->id) REL(r->id);
    r->id = RET(id);
}

void crect_set_masked(crect_t* r, char masked)
{
    r->masked = masked;
}

void crect_set_unmask(crect_t* r, char unmask)
{
    r->unmask = unmask;
}

void crect_set_frame(crect_t* r, r2_t rect)
{
    r->frame = rect;

    r->data[0] = rect.x;
    r->data[1] = rect.y;

    r->data[6] = rect.x + rect.w;
    r->data[7] = rect.y + rect.h;

    r->data[12] = rect.x;
    r->data[13] = rect.y + rect.h;

    r->data[18] = rect.x + rect.w;
    r->data[19] = rect.y;

    r->data[24] = rect.x;
    r->data[25] = rect.y;

    r->data[30] = rect.x + rect.w;
    r->data[31] = rect.y + rect.h;
}

void crect_set_texture(crect_t* r, float tlx, float tly, float brx, float bry)
{
    r->data[2] = tlx;
    r->data[3] = tly;

    r->data[8] = brx;
    r->data[9] = bry;

    r->data[14] = tlx;
    r->data[15] = bry;

    r->data[20] = brx;
    r->data[21] = tly;

    r->data[26] = tlx;
    r->data[27] = tly;

    r->data[32] = brx;
    r->data[33] = bry;
}

void crect_set_page(crect_t* r, uint32_t page)
{
    r->data[4]  = (float) page;
    r->data[10] = (float) page;
    r->data[16] = (float) page;
    r->data[22] = (float) page;
    r->data[28] = (float) page;
    r->data[34] = (float) page;
}

void crect_set_alpha(crect_t* r, float alpha)
{
    r->data[5]  = alpha;
    r->data[11] = alpha;
    r->data[17] = alpha;
    r->data[23] = alpha;
    r->data[29] = alpha;
    r->data[35] = alpha;
}

void crect_desc(void* p, int level)
{
    printf("crect\n");
    crect_t* r = p;
    for (int index = 0; index < 30; index++)
    {
	if (index % 5 == 0) printf("\n");
	printf("%f ", r->data[index]);
    }
}

#endif
