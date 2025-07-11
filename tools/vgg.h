/* vgg.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) vector graphics generator (VGG).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef VGG_H
#define VGG_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define VGG_INLINE inline
#define VGG_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define VGG_INLINE __inline__
#define VGG_API static
#elif defined(_MSC_VER)
#define VGG_INLINE __inline
#define VGG_API static
#else
#define VGG_INLINE
#define VGG_API static
#endif

typedef struct vgg_svg_color
{
  unsigned char r, g, b;
} vgg_svg_color;

typedef struct vgg_svg_writer
{
  unsigned char *buffer;
  int capacity;
  int length;
} vgg_svg_writer;

/* Write a string literal to the buffer */
VGG_API VGG_INLINE void vgg_svg_puts(vgg_svg_writer *w, const char *s)
{
  while (*s && w->length < w->capacity)
  {
    w->buffer[w->length++] = (unsigned char)*s++;
  }
}

/* Write a single character */
VGG_API VGG_INLINE void vgg_svg_putc(vgg_svg_writer *w, char c)
{
  if (w->length < w->capacity)
  {
    w->buffer[w->length++] = (unsigned char)c;
  }
}

/* Write an unsigned int */
VGG_API VGG_INLINE void vgg_svg_put_uint(vgg_svg_writer *w, unsigned int val)
{
  char buf[10];
  int i = 10;
  if (val == 0)
  {
    vgg_svg_putc(w, '0');
    return;
  }
  while (val && i)
  {
    buf[--i] = '0' + (char)(val % 10);
    val /= 10;
  }
  while (i < 10)
  {
    vgg_svg_putc(w, buf[i++]);
  }
}

/* Write a float with 3-digit decimal precision */
VGG_API VGG_INLINE void vgg_svg_put_double(vgg_svg_writer *w, double d)
{
  int i = (int)d;
  int frac;

  if (d < 0)
  {
    vgg_svg_putc(w, '-');
    d = -d;
    i = -i;
  }

  vgg_svg_put_uint(w, (unsigned int)i);
  vgg_svg_putc(w, '.');

  frac = (int)((d - (double)i) * 1000.0);
  if (frac < 0)
  {
    frac = -frac;
  }
  if (frac < 100)
  {
    vgg_svg_putc(w, '0');
  }
  if (frac < 10)
  {
    vgg_svg_putc(w, '0');
  }
  vgg_svg_put_uint(w, (unsigned int)frac);
}

VGG_API VGG_INLINE void vgg_svg_start(vgg_svg_writer *w, double width, double height)
{
  vgg_svg_puts(w, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"");
  vgg_svg_put_double(w, width);
  vgg_svg_puts(w, "\" height=\"");
  vgg_svg_put_double(w, height);
  vgg_svg_puts(w, "\">\n");
}

VGG_API VGG_INLINE void vgg_svg_end(vgg_svg_writer *w)
{
  vgg_svg_puts(w, "</svg>\n");
}

VGG_API VGG_INLINE void vgg_svg_add_rect(
    vgg_svg_writer *w,
    unsigned int rect_id,
    double x, double y, double width, double height,
    vgg_svg_color fill)
{
  static const char hex[] = "0123456789ABCDEF";
  int i;
  char color[7];

  color[0] = hex[(fill.r >> 4) & 0xF];
  color[1] = hex[fill.r & 0xF];
  color[2] = hex[(fill.g >> 4) & 0xF];
  color[3] = hex[fill.g & 0xF];
  color[4] = hex[(fill.b >> 4) & 0xF];
  color[5] = hex[fill.b & 0xF];
  color[6] = 0;

  vgg_svg_puts(w, "<rect id=\"");
  vgg_svg_put_uint(w, rect_id);
  vgg_svg_puts(w, "\" x=\"");
  vgg_svg_put_double(w, x);
  vgg_svg_puts(w, "\" y=\"");
  vgg_svg_put_double(w, y);
  vgg_svg_puts(w, "\" width=\"");
  vgg_svg_put_double(w, width);
  vgg_svg_puts(w, "\" height=\"");
  vgg_svg_put_double(w, height);
  vgg_svg_puts(w, "\" fill=\"#");
  for (i = 0; i < 6; ++i)
  {
    vgg_svg_putc(w, color[i]);
  }
  vgg_svg_puts(w, "\" />\n");
}

#endif /* VGG_H */

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
