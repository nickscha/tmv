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

typedef struct vgg_color
{
  int r;
  int g;
  int b;

} vgg_color;

typedef struct vgg_data_field
{
  char *key;
  char *value;

} vgg_data_field;

typedef enum vgg_header_type
{
  VGG_TYPE_RECT

} vgg_header_type;

typedef struct vgg_header
{
  vgg_header_type type;

  unsigned int id;

  vgg_data_field *data_fields;
  unsigned int data_fields_count;

  vgg_color color_fill;

} vgg_header;

typedef struct vgg_rect
{
  vgg_header header;

  double x;
  double y;
  double width;
  double height;

} vgg_rect;

typedef struct vgg_svg_writer
{
  unsigned char *buffer;
  int capacity;
  int length;

} vgg_svg_writer;

VGG_API VGG_INLINE vgg_color vgg_color_map_linear(
    double value_current,
    double value_min,
    double value_max,
    vgg_color color_start,
    vgg_color color_end)
{
  vgg_color color;

  /* Clamp and normalize the weight */
  double t = 0.0;
  if (value_max > value_min)
  {
    t = (value_current - value_min) / (value_max - value_min);
    if (t < 0.0)
    {
      t = 0.0;
    }
    if (t > 1.0)
    {
      t = 1.0;
    }
  }

  color.r = (int)(color_start.r + t * (color_end.r - color_start.r));
  color.g = (int)(color_start.g + t * (color_end.g - color_start.g));
  color.b = (int)(color_start.b + t * (color_end.b - color_start.b));

  return color;
}

/* Convert datatypes to string */

/* Write integer value as decimal string into buffer.
   buffer must have enough space.
   Returns pointer to buffer.
*/
VGG_API VGG_INLINE char *vgg_itoa(int value, char *buffer)
{
  char temp[12]; /* enough for 32-bit int */
  int i = 0, j, sign;

  if (value == 0)
  {
    buffer[0] = '0';
    buffer[1] = 0;
    return buffer;
  }

  sign = (value < 0) ? -1 : 1;
  if (sign == -1)
  {
    value = -value;
  }

  while (value != 0)
  {
    temp[i++] = (char)('0' + (value % 10));
    value /= 10;
  }
  if (sign == -1)
  {
    temp[i++] = '-';
  }

  /* reverse */
  for (j = 0; j < i; ++j)
  {
    buffer[j] = temp[i - j - 1];
  }
  buffer[i] = 0;
  return buffer;
}

/* Same for long */
VGG_API VGG_INLINE char *vgg_ltoa(long value, char *buffer)
{
  char temp[20]; /* enough for 64-bit long */
  int i = 0, j;
  int sign = (value < 0) ? -1 : 1;

  if (value == 0)
  {
    buffer[0] = '0';
    buffer[1] = 0;
    return buffer;
  }

  if (sign == -1)
    value = -value;

  while (value != 0)
  {
    temp[i++] = (char)('0' + (value % 10));
    value /= 10;
  }
  if (sign == -1)
  {
    temp[i++] = '-';
  }

  for (j = 0; j < i; ++j)
  {
    buffer[j] = temp[i - j - 1];
  }
  buffer[i] = 0;
  return buffer;
}

/* Unsigned long to string */
VGG_API VGG_INLINE char *vgg_ultoa(unsigned long value, char *buffer)
{
  char temp[20];
  int i = 0, j;

  if (value == 0)
  {
    buffer[0] = '0';
    buffer[1] = 0;
    return buffer;
  }

  while (value != 0)
  {
    temp[i++] = (char)('0' + (value % 10));
    value /= 10;
  }

  for (j = 0; j < i; ++j)
  {
    buffer[j] = temp[i - j - 1];
  }
  buffer[i] = 0;
  return buffer;
}

VGG_API VGG_INLINE char *vgg_ftoa(double value, char *buffer, int precision)
{
  int int_part, frac_part, i = 0, j;
  int multiplier = 1;

  if (precision < 0)
  {
    precision = 0;
  }
  if (precision > 9)
  {
    precision = 9; /* prevent overflow */
  }

  /* Sign */
  if (value < 0)
  {
    value = -value;
    buffer[i++] = '-';
  }

  /* Integer part */
  int_part = (int)value;
  vgg_itoa(int_part, buffer + i);

  /* Advance i to end of int_part string */
  while (buffer[i] != '\0')
  {
    i++;
  }

  if (precision == 0)
  {
    buffer[i] = '\0';
    return buffer;
  }

  buffer[i++] = '.';

  /* Calculate fractional part multiplier */
  for (j = 0; j < precision; ++j)
    multiplier *= 10;

  /* Extract fractional part */
  frac_part = (int)((value - int_part) * multiplier + 0.5); /* round */

  /* Write zero-padded fractional part */
  {
    char temp[10];
    int k = 0;

    if (frac_part == 0)
    {
      for (j = 0; j < precision; ++j)
        buffer[i++] = '0';
    }
    else
    {
      /* Build fractional string backwards */
      while (frac_part > 0 || k < precision)
      {
        temp[k++] = (char)('0' + (frac_part % 10));
        frac_part /= 10;
      }

      /* Zero padding if needed */
      while (k < precision)
      {
        temp[k++] = '0';
      }

      /* Copy reversed string to buffer */
      while (k > 0)
      {
        buffer[i++] = temp[--k];
      }
    }
  }

  buffer[i] = '\0';
  return buffer;
}

VGG_API VGG_INLINE vgg_data_field vgg_data_field_create_int(char *key, int value, char *value_buffer)
{
  vgg_data_field df;
  df.key = key;
  vgg_itoa(value, value_buffer);
  df.value = value_buffer;
  return df;
}

VGG_API VGG_INLINE vgg_data_field vgg_data_field_create_long(char *key, long value, char *value_buffer)
{
  vgg_data_field df;
  df.key = key;
  vgg_ltoa(value, value_buffer);
  df.value = value_buffer;
  return df;
}

VGG_API VGG_INLINE vgg_data_field vgg_data_field_create_unsigned_long(char *key, unsigned long value, char *value_buffer)
{
  vgg_data_field df;
  df.key = key;
  vgg_ultoa(value, value_buffer);
  df.value = value_buffer;
  return df;
}

VGG_API VGG_INLINE vgg_data_field vgg_data_field_create_float(char *key, float value, int precision, char *value_buffer)
{
  vgg_data_field df;
  df.key = key;
  vgg_ftoa((double)value, value_buffer, precision);
  df.value = value_buffer;
  return df;
}

VGG_API VGG_INLINE vgg_data_field vgg_data_field_create_double(char *key, double value, int precision, char *value_buffer)
{
  vgg_data_field df;
  df.key = key;
  vgg_ftoa(value, value_buffer, precision);
  df.value = value_buffer;
  return df;
}

/* Write a string literal to the buffer */
VGG_API VGG_INLINE void vgg_svg_puts(vgg_svg_writer *w, char *s)
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

VGG_API VGG_INLINE void vgg_svg_start(vgg_svg_writer *w, char *id, double width, double height)
{
  vgg_svg_puts(w, "<svg ");

  /* ID field */
  vgg_svg_puts(w, "id=\"");
  vgg_svg_puts(w, id);
  vgg_svg_puts(w, "\" ");

  /* XMLNS */
  vgg_svg_puts(w, "xmlns=\"http://www.w3.org/2000/svg\" ");

  /* Width of svg */
  vgg_svg_puts(w, "width=\"");
  vgg_svg_put_double(w, width);
  vgg_svg_puts(w, "\" ");

  /* Height of svg */
  vgg_svg_puts(w, "height=\"");
  vgg_svg_put_double(w, height);
  vgg_svg_puts(w, "\">\n");
}

VGG_API VGG_INLINE void vgg_svg_end(vgg_svg_writer *w)
{
  vgg_svg_puts(w, "</svg>\n");
}

#define VGG_SVG_PUT_COLOR(w, color)               \
  do                                              \
  {                                               \
    static const char hex[] = "0123456789ABCDEF"; \
    char buf[6];                                  \
    int i;                                        \
    buf[0] = hex[(color.r >> 4) & 0xF];           \
    buf[1] = hex[(color.r) & 0xF];                \
    buf[2] = hex[(color.g >> 4) & 0xF];           \
    buf[3] = hex[(color.g) & 0xF];                \
    buf[4] = hex[(color.b >> 4) & 0xF];           \
    buf[5] = hex[(color.b) & 0xF];                \
    for (i = 0; i < 6; ++i)                       \
    {                                             \
      vgg_svg_putc((w), buf[i]);                  \
    }                                             \
  } while (0)

VGG_API VGG_INLINE void vgg_svg_element_add(
    vgg_svg_writer *w,
    vgg_header *header)
{
  unsigned int i;

  /* Open element tag */
  vgg_svg_puts(w, "  <");

  /* Rectangular Shape */
  if (header->type == VGG_TYPE_RECT)
  {
    vgg_rect *rect = (vgg_rect *)header;

    vgg_svg_puts(w, "rect ");

    /* Rect x value */
    vgg_svg_puts(w, "x=\"");
    vgg_svg_put_double(w, rect->x);
    vgg_svg_puts(w, "\" ");

    /* Rect y value */
    vgg_svg_puts(w, "y=\"");
    vgg_svg_put_double(w, rect->y);
    vgg_svg_puts(w, "\" ");

    /* Rect width value */
    vgg_svg_puts(w, "width=\"");
    vgg_svg_put_double(w, rect->width);
    vgg_svg_puts(w, "\" ");

    /* Rect height value */
    vgg_svg_puts(w, "height=\"");
    vgg_svg_put_double(w, rect->height);
    vgg_svg_puts(w, "\" ");
  }

  /* ID field */
  vgg_svg_puts(w, "id=\"");
  vgg_svg_put_uint(w, header->id);
  vgg_svg_puts(w, "\" ");

  /* Fill color */
  vgg_svg_puts(w, "fill=\"#");
  VGG_SVG_PUT_COLOR(w, header->color_fill);
  vgg_svg_puts(w, "\" ");

  /* Data fields */
  for (i = 0; i < header->data_fields_count; ++i)
  {
    vgg_svg_puts(w, "data-");
    vgg_svg_puts(w, header->data_fields[i].key);
    vgg_svg_puts(w, "=\"");
    vgg_svg_puts(w, header->data_fields[i].value);
    vgg_svg_puts(w, "\" ");
  }

  /* Closing element tag */
  vgg_svg_puts(w, "/>\n");
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
