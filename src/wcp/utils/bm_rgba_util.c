#ifndef bm_rgba_util_h
#define bm_rgba_util_h

#include "zc_bm_rgba.c"
#include <string.h>

bm_rgba_t* bm_rgba_new_flip_y(bm_rgba_t* bm);
void       bm_rgba_write(bm_rgba_t* bm, char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

bm_rgba_t* bm_rgba_new_flip_y(bm_rgba_t* bm)
{
  bm_rgba_t* tmp = bm_rgba_new(bm->w, bm->h);
  for (int y = 0; y < bm->h; y++)
  {
    int src_y = bm->h - y - 1;
    memcpy(tmp->data + y * bm->w * 4, bm->data + src_y * bm->w * 4, bm->w * 4);
  }
  return tmp;
}

void bm_rgba_write(bm_rgba_t* bm, char* path)
{
  int w = bm->w;
  int h = bm->h;

  FILE*          f;
  unsigned char* img      = NULL;
  int            filesize = 54 + 3 * w * h; // w is your image width, h is image height, both int

  img = (unsigned char*)malloc(3 * w * h);
  memset(img, 0, 3 * w * h);

  for (int i = 0; i < w; i++)
  {
    for (int j = 0; j < h; j++)
    {
      int index = j * w * 4 + i * 4;

      int x = i;
      int y = j;

      int r = bm->data[index];
      int g = bm->data[index + 1];
      int b = bm->data[index + 2];

      if (r > 255) r = 255;
      if (g > 255) g = 255;
      if (b > 255) b = 255;

      img[(x + y * w) * 3 + 2] = (unsigned char)(r);
      img[(x + y * w) * 3 + 1] = (unsigned char)(g);
      img[(x + y * w) * 3 + 0] = (unsigned char)(b);
    }
  }

  unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
  unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
  unsigned char bmppad[3]         = {0, 0, 0};

  bmpfileheader[2] = (unsigned char)(filesize);
  bmpfileheader[3] = (unsigned char)(filesize >> 8);
  bmpfileheader[4] = (unsigned char)(filesize >> 16);
  bmpfileheader[5] = (unsigned char)(filesize >> 24);

  bmpinfoheader[4]  = (unsigned char)(w);
  bmpinfoheader[5]  = (unsigned char)(w >> 8);
  bmpinfoheader[6]  = (unsigned char)(w >> 16);
  bmpinfoheader[7]  = (unsigned char)(w >> 24);
  bmpinfoheader[8]  = (unsigned char)(h);
  bmpinfoheader[9]  = (unsigned char)(h >> 8);
  bmpinfoheader[10] = (unsigned char)(h >> 16);
  bmpinfoheader[11] = (unsigned char)(h >> 24);

  f = fopen(path, "wb");
  fwrite(bmpfileheader, 1, 14, f);
  fwrite(bmpinfoheader, 1, 40, f);
  for (int i = 0; i < h; i++)
  {
    fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
    fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
  }

  free(img);
  fclose(f);
}

#endif
