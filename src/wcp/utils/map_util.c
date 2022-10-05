#ifndef zc_maputil_h
#define zc_maputil_h

#include "zc_cstring.c"
#include "zc_map.c"

typedef struct _mpair_t
{
    char* key;
    char* value;
} mpair_t;

map_t* mapu_pair(mpair_t pair);

#endif

#if __INCLUDE_LEVEL__ == 0

map_t* mapu_pair(mpair_t pair)
{
    map_t* result = MNEW();
    char*  str    = cstr_new_cstring(pair.value); // REL 0
    MPUT(result, pair.key, str);
    REL(str); // REL 0
    REL(pair.value);
    return result;
}

#endif
