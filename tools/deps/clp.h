/* clp.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) command line parser (CLP).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef CLP_H
#define CLP_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define CLP_INLINE inline
#define CLP_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define CLP_INLINE __inline__
#define CLP_API static
#elif defined(_MSC_VER)
#define CLP_INLINE __inline
#define CLP_API static
#else
#define CLP_INLINE
#define CLP_API static
#endif

#define CLP_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef enum
{
  FLAG_BOOL,
  FLAG_UNSIGNED_LONG,
  FLAG_STRING

} clp_flag_type;

typedef struct
{
  clp_flag_type type;

  char *name;
  void *value;
  void *def_value;
  int maxlen;

} clp_flag;

CLP_API CLP_INLINE int clp_string_compare(const char *a, const char *b)
{
  int i = 0;
  while (a[i] && b[i])
  {
    if (a[i] != b[i])
    {
      return (a[i] < b[i]) ? -1 : 1;
    }

    i++;
  }
  if (a[i] == b[i])
  {
    return 0;
  }
  return (a[i] < b[i]) ? -1 : 1;
}

CLP_API CLP_INLINE unsigned long clp_string_to_ul(const char *s)
{
  unsigned long result = 0;
  int i = 0;
  while (s[i])
  {
    char c = s[i];
    if (c < '0' || c > '9')
    {
      break;
    }
    result = result * 10 + (unsigned long)(c - '0');
    i++;
  }
  return result;
}

CLP_API CLP_INLINE void clp_string_copy(char *dst, const char *src, int maxlen)
{
  int i;
  for (i = 0; i < maxlen - 1 && src[i]; ++i)
  {
    dst[i] = src[i];
  }
  dst[i] = '\0';
}

CLP_API CLP_INLINE void clp_process(clp_flag *flags, int flags_size, char **argv, int argc)
{
  int i, j;

  for (i = 1; i < argc; ++i)
  {
    char *arg = argv[i];

    if (arg[0] == '-' && arg[1] == '-')
    {
      char *eq = 0;
      char *name;
      char *val;

      /* find '=' if any */
      for (j = 2; arg[j]; ++j)
      {
        if (arg[j] == '=')
        {
          eq = arg + j;
          break;
        }
      }

      if (eq)
      {
        *eq = '\0';
        name = arg + 2;
        val = eq + 1;
      }
      else
      {
        name = arg + 2;
        val = 0;
      }

      /* find matching flag */
      for (j = 0; j < flags_size; ++j)
      {
        clp_flag *f = &flags[j];
        /* compare name and f->name */
        if (clp_string_compare(name, f->name) == 0)
        {
          switch (f->type)
          {
          case FLAG_STRING:
            if (val)
            {
              clp_string_copy((char *)f->value, val, f->maxlen);
            }
            break;

          case FLAG_UNSIGNED_LONG:
            if (val)
            {
              unsigned long v = clp_string_to_ul(val);
              *(unsigned long *)f->value = v;
            }
            break;

          case FLAG_BOOL:
            if (!val)
            {
              /* flag present without value = true */
              *(int *)f->value = 1;
            }
            else
            {
              if (clp_string_compare(val, "true") == 0 || clp_string_compare(val, "1") == 0)
              {
                *(int *)f->value = 1;
              }
              else if (clp_string_compare(val, "false") == 0 || clp_string_compare(val, "0") == 0)
              {
                *(int *)f->value = 0;
              }
            }
            break;

          default:
            break;
          }
        }
      }

      if (eq)
        *eq = '=';
    }
  }

  /* Apply defaults */
  for (j = 0; j < flags_size; ++j)
  {
    clp_flag *f = &flags[j];
    switch (f->type)
    {
    case FLAG_STRING:
    {
      char *dst = (char *)f->value;
      if (dst[0] == '\0' && f->def_value)
      {
        clp_string_copy(dst, (char *)f->def_value, f->maxlen);
      }
      break;
    }
    case FLAG_UNSIGNED_LONG:
    {
      unsigned long *dst = (unsigned long *)f->value;
      if (*dst == 0 && f->def_value)
      {
        *dst = *(unsigned long *)f->def_value;
      }
      break;
    }
    case FLAG_BOOL:
    {
      int *dst = (int *)f->value;
      if (*dst == -1 && f->def_value)
      {
        *dst = *(int *)f->def_value;
      }
      else if (*dst == -1)
      {
        *dst = 0; /* default to false if no default */
      }
      break;
    }
    default:
      break;
    }
  }
}

#endif /* CLP_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
