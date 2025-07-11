/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef TMV_H
#define TMV_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define TMV_INLINE inline
#define TMV_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_INLINE __inline__
#define TMV_API static
#elif defined(_MSC_VER)
#define TMV_INLINE __inline
#define TMV_API static
#else
#define TMV_INLINE
#define TMV_API static
#endif

#define TMV_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct tmv_treemap_item
{
  int id;
  double weight;

  /* Hierarchy */
  struct tmv_treemap_item *children;
  int children_count;

} tmv_treemap_item;

typedef struct tmv_treemap_rect
{
  int id;
  double x;
  double y;
  double width;
  double height;

} tmv_treemap_rect;

TMV_API TMV_INLINE tmv_treemap_item *tmv_find_item_by_id(tmv_treemap_item *items, int count, int id)
{
  int i;
  for (i = 0; i < count; ++i)
  {
    if (items[i].id == id)
    {
      return &items[i];
    }

    if (items[i].children && items[i].children_count > 0)
    {
      tmv_treemap_item *found = tmv_find_item_by_id(items[i].children, items[i].children_count, id);
      if (found != 0)
      {
        return found;
      }
    }
  }

  return 0;
}

TMV_API TMV_INLINE double tmv_total_weight(tmv_treemap_item *items, int count)
{
  double sum = 0.0;
  int i;
  for (i = 0; i < count; ++i)
  {
    sum += items[i].weight;
  }
  return sum;
}

TMV_API TMV_INLINE void tmv_layout_row(
    tmv_treemap_item *row_items,
    int row_count,
    double x,
    double y,
    double width,
    double height,
    tmv_treemap_rect *rects,
    int *rects_count)
{
  int i;
  double total_weight = tmv_total_weight(row_items, row_count);
  double area = width * height;

  int horizontal = (width >= height);

  double offset = 0.0;

  for (i = 0; i < row_count; ++i)
  {
    double item_area = (row_items[i].weight / total_weight) * area;
    double w, h;

    if (horizontal)
    {
      w = item_area / height;
      h = height;
      rects[*rects_count].x = x + offset;
      rects[*rects_count].y = y;
      rects[*rects_count].width = w;
      rects[*rects_count].height = h;
      offset += w;
    }
    else
    {
      w = width;
      h = item_area / width;
      rects[*rects_count].x = x;
      rects[*rects_count].y = y + offset;
      rects[*rects_count].width = w;
      rects[*rects_count].height = h;
      offset += h;
    }

    rects[*rects_count].id = row_items[i].id;
    (*rects_count)++;
  }
}

TMV_API TMV_INLINE void tmv_squarify_current(
    tmv_treemap_item *items, /* The descending by weight sorted treemap items*/
    int items_count,         /* The number of items */
    double x,                /* x */
    double y,                /* y */
    double width,            /* The width for the treemap */
    double height,           /* The height for the treemap */
    tmv_treemap_rect *rects, /* The output rects that have been computed */
    int *rects_count         /* The number of output rects computed */
)
{
  int start = 0;

  double total_weight = tmv_total_weight(items, items_count);
  double area = width * height;
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  while (start < items_count)
  {
    int end = start;
    double row_weight = 0.0;
    double worst = 1e9;
    int i;

    double side = (width >= height) ? height : width;

    double row_length;
    int row_count;

    /* Try to add items[start..end] */
    while (end < items_count)
    {
      double max_w = -1e9;
      double min_w = 1e9;
      double w_scaled;

      double row_area;
      double r1;
      double r2;
      double new_worst;

      row_weight += items[end].weight;

      /* Compute scaled weights and track min/max */
      for (i = start; i <= end; ++i)
      {
        w_scaled = items[i].weight * scale;
        if (w_scaled > max_w)
        {
          max_w = w_scaled;
        }
        if (w_scaled < min_w)
        {
          min_w = w_scaled;
        }
      }

      /* Calculate the new worst aspect ratio s*/
      row_area = row_weight * scale;
      r1 = (side * side * max_w) / (row_area * row_area);
      r2 = (row_area * row_area) / (side * side * min_w);
      new_worst = (r1 > r2) ? r1 : r2;

      /* Stop if aspect ratio would worsen */
      if (new_worst > worst)
      {
        row_weight -= items[end].weight;
        break;
      }

      worst = new_worst;
      ++end;
    }

    /* Compute row size in layout direction */
    row_length = (row_weight / total_weight) * (area / side);
    row_count = end - start;

    if (width >= height)
    {
      tmv_layout_row(&items[start], row_count, x, y, row_length, height, rects, rects_count);
      x += row_length;
      width -= row_length;
    }
    else
    {
      tmv_layout_row(&items[start], row_count, x, y, width, row_length, rects, rects_count);
      y += row_length;
      height -= row_length;
    }

    start = end;
  }
}

TMV_API TMV_INLINE void tmv_squarify(
    tmv_treemap_item *items, /* The descending by weight sorted treemap items*/
    int items_count,         /* The number of items */
    double x,                /* x */
    double y,                /* y */
    double width,            /* The width for the treemap */
    double height,           /* The height for the treemap */
    tmv_treemap_rect *rects, /* The output rects that have been computed */
    int *rects_count         /* The number of output rects computed */
)
{
  int i;
  int j;

  if (items_count == 0)
  {
    return;
  }

  /* Lay out the current level */
  tmv_squarify_current(items, items_count, x, y, width, height, rects, rects_count);

  /* For each item, recurse into children if any */
  for (i = *rects_count - items_count, j = 0; j < items_count; ++j, ++i)
  {
    tmv_treemap_item *item = &items[j];

    if (item->children_count > 0)
    {
      tmv_treemap_rect *parent_rect = &rects[i];

      tmv_squarify(
          item->children,
          item->children_count,
          parent_rect->x,
          parent_rect->y,
          parent_rect->width,
          parent_rect->height,
          rects,
          rects_count);
    }
  }
}

#endif /* TMV_H */

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
