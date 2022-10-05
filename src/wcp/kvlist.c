#ifndef kvlist_h
#define kvlist_h

#include "zc_map.c"
#include <stdio.h>

int kvlist_read(char* libpath, map_t* db, char* keyfield);
int kvlist_write(char* libpath, map_t* db);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cstr_util.c"
#include "zc_cstring.c"
#include "zc_log.c"
#include <limits.h>

int kvlist_read(char* libpath, map_t* db, char* keyfield)
{
  int   retv  = -1;
  char* dbstr = cstr_new_file(libpath); // REL 0

  if (dbstr)
  {
    retv = 0;

    char*  token = strtok(dbstr, "\n");
    char*  key   = NULL;
    map_t* map   = MNEW(); // REL 1

    while (token)
    {
      if (key)
      {
        char* val = cstr_new_cstring(token);
        MPUT(map, key, val);
        REL(key);
        REL(val);
        key = NULL;
      }
      else
      {
        if (token[0] == '-')
        {
          key = MGET(map, keyfield);
          MPUT(db, key, map);
          REL(map);     // REL 1
          map = MNEW(); // REL 1
          key = NULL;
        }
        else
          key = cstr_new_cstring(token);
      }
      token = strtok(NULL, "\n");
    }

    REL(map);   // REL 1
    REL(dbstr); // REL 0
  }
  else
    zc_log_debug("kvlist_read cannot read file %s", libpath);

  return retv;
}

int kvlist_write(char* libpath, map_t* db)
{
  int   retv = -1;
  char* path = cstr_new_format(PATH_MAX + NAME_MAX, "%snew", libpath); // REL 0
  FILE* file = fopen(path, "w");                                       // CLOSE 0

  if (file)
  {
    retv        = 0;
    vec_t* vals = VNEW(); // REL 1
    map_values(db, vals);

    for (int vali = 0; vali < vals->length; vali++)
    {
      map_t* entry = vals->data[vali];
      vec_t* keys  = VNEW(); // REL 2

      map_keys(entry, keys);

      for (int keyi = 0; keyi < keys->length; keyi++)
      {
        char* key = keys->data[keyi];
        char* val = MGET(entry, key);

        if (fprintf(file, "%s\n", key) < 0) retv = -1;
        if (fprintf(file, "%s\n", val) < 0) retv = -1;
      }

      if (fprintf(file, "-\n") < 0) retv = -1;

      REL(keys); // REL 2

      if (retv < 0) break;
    }

    if (fclose(file) == EOF) retv = -1; // CLOSE 0

    REL(vals); // REL 1

    if (retv == 0)
    {
      if (rename(path, libpath) != 0) retv = -1;
    }
    else
      zc_log_error("ERROR kvlist_write cannot write file");
  }
  else
    zc_log_error("ERROR kvlist_write cannot open file %s", path);

  REL(path); // REL 0

  return retv;
}

#endif
