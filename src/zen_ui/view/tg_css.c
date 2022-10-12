/*
  CSS texture generator
  Generates texture based on css style
 */

#ifndef texgen_css_h
#define texgen_css_h

#include "view.c"
#include "zc_bm_rgba.c"

typedef struct _tg_css_t
{
    char*      id;
    char*      path;
    bm_rgba_t* bitmap;
} tg_bitmap_t;

void tg_css_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_log.c"

#define PNG_DEBUG 3
#include <png.h>

uint32_t tg_css_graycolor = 0;

void abort_(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void tg_css_gen(view_t* view)
{
    if (view->frame.local.w >= 1.0 &&
	view->frame.local.h >= 1.0)
    {
	if (view->style.background_image)
	{
	    bm_rgba_t* bm = view->texture.bitmap;

	    if (bm == NULL ||
		bm->w != (int) view->frame.local.w ||
		bm->h != (int) view->frame.local.h)
	    {
		bm = bm_rgba_new((int) view->frame.local.w, (int) view->frame.local.h); // REL 0
		view_set_texture_bmp(view, bm);
		REL(bm);
	    }

	    /* coder_load_image_into(view->style.background_image, view->texture.bitmap); */
	    view->texture.changed = 0;
	    view->texture.state   = TS_READY;

	    /* bm_rgba_t* bmap = coder_get_image(view->style.background_image); */
	    /* view_set_texture_bmp(view, bmap); */
	    /* REL(bmap); */

	    char* file_name = view->style.background_image;

	    int y;

	    int width, height;

	    png_structp png_ptr;
	    png_infop   info_ptr;
	    png_bytep*  row_pointers;

	    unsigned char header[8]; // 8 is the maximum size that can be checked

	    /* open file and test for it being a png */
	    FILE* fp = fopen(file_name, "rb");
	    if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading", file_name);
	    fread(header, 1, 8, fp);
	    if (png_sig_cmp(header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);

	    /* initialize stuff */
	    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	    if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	    info_ptr = png_create_info_struct(png_ptr);
	    if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	    if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during init_io");

	    png_init_io(png_ptr, fp);
	    png_set_sig_bytes(png_ptr, 8);

	    png_read_info(png_ptr, info_ptr);

	    width  = png_get_image_width(png_ptr, info_ptr);
	    height = png_get_image_height(png_ptr, info_ptr);

	    png_read_update_info(png_ptr, info_ptr);

	    /* read file */
	    if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during read_image");

	    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	    for (y = 0; y < height; y++)
		row_pointers[y] = (png_byte*) malloc(rowbytes);

	    png_read_image(png_ptr, row_pointers);

	    bm_rgba_t* rawbm = bm_rgba_new(width, height); // REL 0

	    // copy to bmp
	    for (y = 0; y < height; y++)
	    {
		memcpy((uint8_t*) rawbm->data + y * width * 4, row_pointers[y], rowbytes);
	    }

	    for (y = 0; y < height; y++)
		free(row_pointers[y]);
	    free(row_pointers);

	    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	    fclose(fp);

	    if (bm->w == rawbm->w && bm->h == rawbm->h)
		gfx_insert(bm, rawbm, 0, 0);
	    else
		gfx_scale(rawbm, bm);

	    REL(rawbm);

	    /* view_set_texture_bmp(view, bm); */

	    /* REL(bm); */
	}
	else if (view->style.background_color)
	{
	    uint32_t color = view->style.background_color;

	    float w = view->frame.local.w + 2 * view->style.shadow_blur;
	    float h = view->frame.local.h + 2 * view->style.shadow_blur;

	    bm_rgba_t* bm = view->texture.bitmap;

	    if (bm == NULL ||
		bm->w != (int) w ||
		bm->h != (int) h)
	    {
		bm = bm_rgba_new((int) w, (int) h); // REL 0
		view_set_texture_bmp(view, bm);
		REL(bm);
	    }

	    bm_rgba_reset(bm);

	    gfx_rounded_rect(bm, 0, 0, w, h, view->style.border_radius, view->style.shadow_blur, color, view->style.shadow_color);

	    view->texture.changed = 1;
	    view->texture.state   = TS_READY;
	}
    }
}

void tg_css_add(view_t* view)
{
    if (view->tex_gen != NULL) zc_log_debug("Text generator already exist for view, cannot create a new one : %s", view->id);
    else
    {
	view->tex_gen = tg_css_gen;
	view->exclude = 0;
    }
}

#endif
