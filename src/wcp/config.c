#ifndef config_h
#define config_h

#include "mt_map.c"

void  config_init();
void  config_destroy();
void  config_read(char* path);
void  config_write(char* path);
void  config_set(char* key, char* value);
char* config_get(char* key);
int   config_get_int(char* key);
int   config_get_bool(char* key);
void  config_set_bool(char* key, int val);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "kvlist.c"
#include "mt_log.c"
#include "mt_path.c"
#include "mt_string.c"
#include <limits.h>

mt_map_t* confmap;

void config_init()
{
    confmap = MNEW(); // REL 0
}

void config_destroy()
{
    REL(confmap); // REL 0
}

void config_set(char* key, char* value)
{
    char* str = mt_string_new_cstring(value); // REL 0
    MPUT(confmap, key, str);
    REL(str); // REL 0
}

char* config_get(char* key)
{
    return MGET(confmap, key);
}

int config_get_bool(char* key)
{
    char* val = MGET(confmap, key);
    if (val && strcmp(val, "true") == 0)
	return 1;
    else
	return 0;
}

int config_get_int(char* key)
{
    char* val = MGET(confmap, key);
    if (val)
	return atoi(val);
    else
	return 0;
}

void config_set_bool(char* key, int val)
{
    if (val)
    {
	MPUTR(confmap, key, mt_string_new_cstring("true"));
    }
    else
    {
	MPUTR(confmap, key, mt_string_new_cstring("false"));
    }
}

#endif
