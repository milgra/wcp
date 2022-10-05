#ifndef texgen_picker_h
#define texgen_picker_h

#include "view.c"

void tg_picker_add(view_t* view);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_draw.c"

void tg_picker_gen(view_t* view)
{
  if (view->frame.local.w > 0 && view->frame.local.h > 0)
  {
    if (view->texture.bitmap == NULL && view->frame.local.w > 0 && view->frame.local.h > 0)
    {
      bm_rgba_t* bmp = bm_rgba_new(view->frame.local.w, view->frame.local.h); // REL 0

      int r  = 0;
      int g  = 0;
      int b  = 0;
      int pw = bmp->w / 6 + 1;

      for (int h = 0; h < bmp->h; h++)
      {
        for (int w = 0; w < bmp->w; w++)
        {

          if (w > 0 && w < pw)
          {
            r = 255;
            g = w * 255 / pw;
            b = 0;
          }
          else if (w >= pw && w < 2 * pw)
          {
            r = (2 * pw - w) * 255 / pw;
            g = 255;
            b = 0;
          }
          else if (w >= 2 * pw && w < 3 * pw)
          {
            r = 0;
            g = 255;
            b = (w - 2 * pw) * 255 / pw;
          }
          else if (w >= 3 * pw && w < 4 * pw)
          {
            r = 0;
            g = (4 * pw - w) * 255 / pw;
            b = 255;
          }
          else if (w >= 4 * pw && w < 5 * pw)
          {
            r = (w - 4 * pw) * 255 / pw;
            g = 0;
            b = 255;
          }
          else if (w >= 5 * pw && w < 6 * pw)
          {
            r = 255;
            g = 0;
            b = (6 * pw - w) * 255 / pw;
          }

          if (h < bmp->h / 2)
          {
            r += (bmp->h / 2 - h) * (255 - r) / (bmp->h / 2);
            g += (bmp->h / 2 - h) * (255 - g) / (bmp->h / 2);
            b += (bmp->h / 2 - h) * (255 - b) / (bmp->h / 2);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
          }
          else
          {
            r -= (h - bmp->h / 2) * r / (bmp->h / 2);
            g -= (h - bmp->h / 2) * g / (bmp->h / 2);
            b -= (h - bmp->h / 2) * b / (bmp->h / 2);
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
          }

          uint32_t col = r << 24 | g << 16 | b << 8 | 0xFF;

          gfx_rect(bmp, w, h, 1, 1, col, 0);
        }
      }

      view_set_texture_bmp(view, bmp);
      REL(bmp);
    }
  }
}

void tg_picker_add(view_t* view)
{
  assert(view->tex_gen == NULL);

  view->tex_gen = tg_picker_gen;
  view->exclude = 0;
}

#endif
