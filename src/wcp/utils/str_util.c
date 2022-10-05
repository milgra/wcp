#ifndef zc_strutil_h
#define zc_strutil_h

#include "zc_string.c"

str_t* str_new_replace(str_t* string, str_t* newstring, int start, int end);
void   str_new_replace_codepoints(str_t* string, uint32_t oldcp, uint32_t newcp);
vec_t* str_split(str_t* string, char character);

int      str_intvalue(str_t* string);
float    str_floatvalue(str_t* string);
uint32_t str_unsignedvalue(str_t* string);

str_t*   str_compactemojis(str_t* string);
uint32_t str_find(str_t* string, str_t* substring, uint32_t from);

#endif

#if __INCLUDE_LEVEL__ == 0

/* replaces substring in string */

str_t* str_new_replace(str_t* string, str_t* newstring, int start, int end)
{
  str_t* part1 = str_new_substring(string, 0, start);
  str_t* part2 = str_new_substring(string, end, string->length);

  str_add_string(part1, newstring);
  str_add_string(part1, part2);

  return part1;
}

/* replaces codepoints */

void str_new_replace_codepoints(str_t* string, uint32_t oldcp, uint32_t newcp)
{
  for (int index = 0; index < string->length; index++)
  {
    if (string->codepoints[index] == oldcp) string->codepoints[index] = newcp;
  }
}

/* splits string at codepoint to a vector */

vec_t* str_split(str_t* string, char codepoint)
{
  vec_t* vector  = VNEW();
  str_t* segment = str_new();
  for (int index = 0; index < string->length; index++)
  {
    if (string->codepoints[index] == codepoint)
    {
      // add word to result, create new word
      if (segment->length > 0)
      {
        vec_add(vector, segment);
        REL(segment);
        segment = str_new();
      }
    }
    else
      str_add_codepoint(segment, string->codepoints[index]);
  }
  // add word to result
  if (segment->length > 0) vec_add(vector, segment);
  REL(segment);
  return vector;
}

/* returns intvalue */

int str_intvalue(str_t* string)
{
  char* viewindexc = str_new_cstring(string);
  int   viewindex  = atoi(viewindexc);
  REL(viewindexc);
  return viewindex;
}

/* returns floatvalue */

float str_floatvalue(str_t* string)
{
  char* viewindexc = str_new_cstring(string);
  float viewindex  = atof(viewindexc);
  REL(viewindexc);
  return viewindex;
}

/* returns unsigned value */

uint32_t str_unsignedvalue(str_t* string)
{
  char*         valuec = str_new_cstring(string);
  unsigned long value  = strtoul(valuec, NULL, 0);
  REL(valuec);
  return (uint32_t)value;
}

/* compact :) and :( to their unicode codepoints */

str_t* str_compactemojis(str_t* string)
{
  str_t* result = str_new();

  for (int index = 0; index < string->length; index++)
  {
    if (index < string->length - 1)
    {
      if (string->codepoints[index] == ':' && string->codepoints[index + 1] == ')')
      {
        str_add_codepoint(result, 0x1F601);
        index++;
      }
      else if (string->codepoints[index] == ':' && string->codepoints[index + 1] == '(')
      {
        str_add_codepoint(result, 0x1F61E);
        index++;
      }
      else
        str_add_codepoint(result, string->codepoints[index]);
    }
    else
      str_add_codepoint(result, string->codepoints[index]);
  }
  return result;
}

/* finds substring in string from given index */

uint32_t str_find(str_t* string, str_t* substring, uint32_t from)
{
  if (string == NULL) return UINT32_MAX;
  if (substring == NULL) return UINT32_MAX;
  if (string->length < substring->length) return UINT32_MAX;

  for (uint32_t index = from; index < string->length - substring->length + 1; index++)
  {
    if (string->codepoints[index] == substring->codepoints[0])
    {
      uint32_t count;
      for (count = 1; count < substring->length; count++)
      {
        if (string->codepoints[index + count] != substring->codepoints[count]) break;
      }
      if (count == substring->length) return index;
    }
  }
  return UINT32_MAX;
}

#endif
