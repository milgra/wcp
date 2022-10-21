#ifndef zc_cstring_h
#define zc_cstring_h

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

char* cstr_new_format(int size, char* format, ...);
char* cstr_new_cstring(char* string);
char* cstr_append(char* str, char* add);
char* cstr_append_sub(char* str, char* add, int from, int len);
char* cstr_new_delete_utf_codepoints(char* str, int from, int len);
void  cstr_describe(void* p, int level);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "utf8.h"
#include "zc_memory.c"
#include <ctype.h>
#include <string.h>

char* cstr_new_format(int size, char* format, ...)
{
    char*   result = CAL(sizeof(char) * size, NULL, cstr_describe);
    va_list args;

    va_start(args, format);
    vsnprintf(result, size, format, args);
    va_end(args);

    return result;
}

char* cstr_new_cstring(char* string)
{
    char* result = NULL;
    if (string != NULL)
    {
	result = CAL((strlen(string) + 1) * sizeof(char), NULL, cstr_describe);
	memcpy(result, string, strlen(string));
    }
    return result;
}

char* cstr_append(char* str, char* add)
{
    size_t needed = strlen(str) + strlen(add) + 1;

    if (strlen(str) < needed) str = mem_realloc(str, needed);
    strcat(str, add);

    return str;
}

char* cstr_append_sub(char* str, char* add, int from, int len)
{
    size_t needed  = strlen(str) + len + 1;
    int    oldsize = strlen(str);

    if (strlen(str) < needed) str = mem_realloc(str, needed);
    memcpy(str + oldsize, add + from, len);
    str[needed - 1] = '\0';

    return str;
}

char* cstr_new_delete_utf_codepoints(char* str, int from, int len)
{
    size_t       count = utf8len(str);
    const void*  part  = str;
    utf8_int32_t cp;
    char*        new_text = CAL(count, NULL, NULL);
    char*        new_part = new_text;

    // remove last codepoint
    for (int index = 0; index < count - 1; index++)
    {
	part = utf8codepoint(part, &cp);
	if (index < from || index > from + len) new_part = utf8catcodepoint(new_part, cp, 4);
    }

    return new_text;
}

void cstr_describe(void* p, int level)
{
    printf("%s", (char*) p);
}

#endif
