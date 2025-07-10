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

TMV_API TMV_INLINE double tmv_worst_aspect(double *weights, int count, double side_length)
{
  int i;

  double total = 0.0;
  double max_w = 0.0;
  double min_w = 1e9;

  double side_sq;
  double r1;
  double r2;

  for (i = 0; i < count; ++i)
  {
    double w = weights[i];

    total += w;
    if (w > max_w)
    {
      max_w = w;
    }
    if (w < min_w)
    {
      min_w = w;
    }
  }

  if (min_w <= 0.0 || side_length <= 0.0)
  {
    return 1e9;
  }

  side_sq = side_length * side_length;
  r1 = (side_sq * max_w) / (total * total);
  r2 = (total * total) / (side_sq * min_w);

  return (r1 > r2) ? r1 : r2;
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

#define MAX_RECTS 512

TMV_API TMV_INLINE void tmv_squarify(
    tmv_treemap_item *items, /* The descending by weight sorted treemap items*/
    int items_count,         /* The number of items */
    double x,                /* x */
    double y,                /* y */
    double width,            /* The width for the treemap */
    double height,           /* The height for the treemap */
    tmv_treemap_rect *rects, /* The output rects that have been computed */
    int *rects_count,        /* The number of output rects computed */
    double total_weight_of_all_items)
{

  int i, row_count = 0;
  double weights[MAX_RECTS];
  tmv_treemap_item row_items[MAX_RECTS];

  int horizontal = (width >= height);
  double side_length = horizontal ? height : width;
  double area = width * height;

  if (items_count == 0)
  {
    return;
  }

  if (items_count == 1)
  {
    rects[*rects_count].id = items[0].id;
    rects[*rects_count].x = x;
    rects[*rects_count].y = y;
    rects[*rects_count].width = width;
    rects[*rects_count].height = height;
    (*rects_count)++;
    return;
  }

  for (i = 0; i < items_count; ++i)
  {
    row_items[row_count] = items[i];
    weights[row_count] = items[i].weight;
    row_count++;

    /* Scale weights to area */
    {
      int j;
      double scaled_weights[MAX_RECTS];
      double worst_with;

      for (j = 0; j < row_count; ++j)
      {
        scaled_weights[j] = (weights[j] / total_weight_of_all_items) * area;
      }

      worst_with = tmv_worst_aspect(scaled_weights, row_count, side_length);

      if (i + 1 < items_count)
      {
        int next_count;
        double worst_with_next;

        weights[row_count] = items[i + 1].weight;
        next_count = row_count + 1;

        for (j = 0; j < next_count; ++j)
        {
          scaled_weights[j] = (weights[j] / total_weight_of_all_items) * area;
        }

        worst_with_next = tmv_worst_aspect(scaled_weights, next_count, side_length);

        if (worst_with_next > worst_with)
        {
          double total_weight = tmv_total_weight(row_items, row_count);
          double row_side_length = (total_weight / total_weight_of_all_items) * (area / side_length);

          if (horizontal)
          {
            tmv_layout_row(row_items, row_count, x, y, row_side_length, height, rects, rects_count);
            tmv_squarify(items + i + 1, items_count - i - 1,
                         x + row_side_length, y, width - row_side_length, height,
                         rects, rects_count, total_weight_of_all_items);
          }
          else
          {
            tmv_layout_row(row_items, row_count, x, y, width, row_side_length, rects, rects_count);
            tmv_squarify(items + i + 1, items_count - i - 1,
                         x, y + row_side_length, width, height - row_side_length,
                         rects, rects_count, total_weight_of_all_items);
          }
          return;
        }
      }
    }
  }

  /* All items fit in one row */
  tmv_layout_row(row_items, row_count, x, y, width, height, rects, rects_count);
}

TMV_API TMV_INLINE void tmv_squarify_recursive(
    tmv_treemap_item *items,
    int items_count,
    double x, double y,
    double width, double height,
    tmv_treemap_rect *rects,
    int *rects_count,
    double total_weight)
{
  int i;
  int j;

  if (items_count == 0)
  {
    return;
  }

  /* Lay out the current level */
  tmv_squarify(items, items_count, x, y, width, height, rects, rects_count, total_weight);

  /* For each item, recurse into children if any */
  for (i = *rects_count - items_count, j = 0; j < items_count; ++j, ++i)
  {
    tmv_treemap_item *item = &items[j];

    if (item->children_count > 0)
    {
      double child_total = tmv_total_weight(item->children, item->children_count);

      tmv_treemap_rect *parent_rect = &rects[i];

      tmv_squarify_recursive(
          item->children,
          item->children_count,
          parent_rect->x,
          parent_rect->y,
          parent_rect->width,
          parent_rect->height,
          rects,
          rects_count,
          child_total);
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
