#ifndef _zc_texmap_h
#define _zc_texmap_h

#include "zc_bm_rgba.c"
#include "zc_map.c"
#include "zc_mat4.c"
#include "zc_vec4.c"

typedef struct _tm_coords_t tm_coords_t;
struct _tm_coords_t
{
  float ltx;
  float lty;
  float rbx;
  float rby;
  int   x;
  int   y;
  int   w;
  int   h;
};

typedef struct _tm_t tm_t;
struct _tm_t
{
  map_t* coords;
  char*  blocks;
  char   is_full;
  int    width;
  int    height;
  int    cols;
  int    rows;
};

tm_t*       tm_new(int w, int h);
void        tm_del(void* p);
char        tm_has(tm_t* tm, char* id);
tm_coords_t tm_get(tm_t* tm, char* id);
int         tm_put(tm_t* tm, char* id, int w, int h);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_memory.c"
#include <math.h>

void tm_desc(void* p, int level)
{
  tm_t* tm = p;
  printf("tm w %i h %i", tm->width, tm->height);
}

void tm_desc_blocks(void* p, int level)
{
  printf("tm blocks\n");
}

tm_t* tm_new(int w, int h)
{
  int cols = w / 32;
  int rows = h / 32;

  tm_t* tm   = CAL(sizeof(tm_t), tm_del, tm_desc);
  tm->coords = map_new();
  tm->blocks = CAL(sizeof(char) * cols * rows, NULL, tm_desc_blocks);
  tm->width  = w;
  tm->height = h;
  tm->cols   = cols;
  tm->rows   = rows;

  return tm;
}

void tm_del(void* p)
{
  tm_t* tm = (tm_t*)p;
  REL(tm->coords);
  REL(tm->blocks);
}

char tm_has(tm_t* tm, char* id)
{
  v4_t* coords = map_get(tm->coords, id);
  if (coords) return 1;
  return 0;
}

tm_coords_t tm_get(tm_t* tm, char* id)
{
  tm_coords_t* coords = map_get(tm->coords, id);
  if (coords) return *coords;
  return (tm_coords_t){0};
}

int tm_put(tm_t* tm, char* id, int w, int h)
{
  if (w > tm->width || h > tm->height) return -1; // too big bitmap

  // get size of incoming rect
  int sx = ceil((float)w / 32.0);
  int sy = ceil((float)h / 32.0);

  int r = 0; // row
  int c = 0; // col
  int s = 0; // success

  for (r = 0; r < tm->rows; r++)
  {
    for (c = 0; c < tm->cols; c++)
    {
      int i = r * tm->cols + c;

      // if block is free, check if width and height of new rect fits

      if (tm->blocks[i] == 0)
      {
        s = 1; // assume success
        if (c + sx < tm->cols)
        {
          for (int tc = c; tc < c + sx; tc++)
          {
            int ti = r * tm->cols + tc; // test index
            if (tm->blocks[ti] == 1)
            {
              s = 0; // if block is occupied test is failed
              break;
            }
          }
        }
        else
          s = 0; // doesn't fit

        if (r + sy < tm->rows && s == 1)
        {
          for (int tr = r; tr < r + sy; tr++)
          {
            int ti = tr * tm->cols + c; // test index
            if (tm->blocks[ti] == 1)
            {
              s = 0; // if block is occupied test is failed
              break;
            }
          }
        }
        else
          s = 0; // doesn't fit
      }
      if (s == 1) break;
    }
    if (s == 1) break;
  }

  if (s == 1)
  {
    // flip blocks to occupied
    for (int nr = r; nr < r + sy; nr++)
    {
      for (int nc = c; nc < c + sx; nc++)
      {
        int ni         = nr * tm->cols + nc;
        tm->blocks[ni] = 1;
      }
    }

    int ncx = c * 32;
    int ncy = r * 32;
    int rbx = ncx + w;
    int rby = ncy + h;

    tm_coords_t* coords = HEAP(((tm_coords_t){.ltx = (float)ncx / (float)tm->width,
                                              .lty = (float)ncy / (float)tm->height,
                                              .rbx = (float)rbx / (float)tm->width,
                                              .rby = (float)rby / (float)tm->height,
                                              .x   = ncx,
                                              .y   = ncy,
                                              .w   = w,
                                              .h   = h}));

    MPUTR(tm->coords, id, coords);
  }
  else
    return -2; // texmap is full

  return 0; // success
}

#endif
