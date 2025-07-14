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

typedef struct tmv_item
{
  unsigned int id;
  double weight;

  void *user_data;

  /* Hierarchy */
  struct tmv_item *children;
  int children_count;

} tmv_item;

typedef struct tmv_rect
{
  unsigned int id;
  double x;
  double y;
  double width;
  double height;

} tmv_rect;

typedef struct tmv_stats
{
  double weigth_min;
  double weigth_max;
  double weigth_sum;
  long count;

} tmv_stats;

TMV_API TMV_INLINE int tmv_total_items(tmv_item *items, int items_count)
{
  int total = 0;
  int i;

  for (i = 0; i < items_count; ++i)
  {
    total += 1;

    if (items[i].children && items[i].children_count > 0)
    {
      total += tmv_total_items(items[i].children, items[i].children_count);
    }
  }

  return total;
}

TMV_API TMV_INLINE tmv_item *tmv_find_item_by_id(tmv_item *items, int count, unsigned int id)
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
      tmv_item *found = tmv_find_item_by_id(items[i].children, items[i].children_count, id);
      if (found != 0)
      {
        return found;
      }
    }
  }

  return 0;
}

TMV_API TMV_INLINE void tmv_insertion_sort_stable_desc(tmv_item *items, int count)
{
  int i, j;
  for (i = 1; i < count; ++i)
  {
    tmv_item key = items[i];
    j = i - 1;

    /* Move elements with weight < key.weight one position forward */
    while (j >= 0 && items[j].weight < key.weight)
    {
      items[j + 1] = items[j];
      j--;
    }
    items[j + 1] = key;
  }
}

TMV_API TMV_INLINE double tmv_total_weight(tmv_item *items, int count)
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
    tmv_rect row_area,
    tmv_item *row_items,
    int row_count,
    tmv_rect *rects,
    int *rects_count,
    tmv_stats *stats)
{
  int i;
  double area = row_area.width * row_area.height;
  double total_weight = tmv_total_weight(row_items, row_count);
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  int horizontal = (row_area.width >= row_area.height);
  double offset = 0.0;

  for (i = 0; i < row_count; ++i)
  {
    double item_area = row_items[i].weight * scale;
    double w, h;

    if (row_items[i].children_count == 0 && stats != 0)
    {
      double w = row_items[i].weight;

      if (stats->weigth_min < 0.0 || w < stats->weigth_min)
      {
        stats->weigth_min = w;
      }

      if (stats->weigth_max < 0.0 || w > stats->weigth_max)
      {
        stats->weigth_max = w;
      }

      stats->weigth_sum += w;
      stats->count += 1;
    }

    if (horizontal)
    {
      w = item_area / row_area.height;
      h = row_area.height;
      rects[*rects_count].x = row_area.x + offset;
      rects[*rects_count].y = row_area.y;
      rects[*rects_count].width = w;
      rects[*rects_count].height = h;
      offset += w;
    }
    else
    {
      w = row_area.width;
      h = item_area / row_area.width;
      rects[*rects_count].x = row_area.x;
      rects[*rects_count].y = row_area.y + offset;
      rects[*rects_count].width = w;
      rects[*rects_count].height = h;
      offset += h;
    }

    rects[*rects_count].id = row_items[i].id;
    (*rects_count)++;
  }
}

TMV_API TMV_INLINE void tmv_squarify_current(
    tmv_rect render_area, /* The area on which the squarified treemap should be aligned */
    tmv_item *items,      /* The descending by weight sorted treemap items*/
    int items_count,      /* The number of items */
    tmv_rect *rects,      /* The output rects that have been computed */
    int *rects_count,     /* The number of output rects computed */
    tmv_stats *stats      /* The output stats/metrics */
)
{
  int start = 0;
  double total_weight = tmv_total_weight(items, items_count);
  double area = render_area.width * render_area.height;
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  tmv_insertion_sort_stable_desc(items, items_count);

  while (start < items_count)
  {
    int end = start;
    double row_weight = 0.0;
    double worst = 1e9;
    int i;

    double side = (render_area.width >= render_area.height) ? render_area.height : render_area.width;

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

    if (render_area.width >= render_area.height)
    {
      tmv_rect row_area = render_area;
      row_area.width = row_length;

      tmv_layout_row(row_area, &items[start], row_count, rects, rects_count, stats);
      render_area.x += row_length;
      render_area.width -= row_length;
    }
    else
    {
      tmv_rect row_area = render_area;
      row_area.height = row_length;

      tmv_layout_row(row_area, &items[start], row_count, rects, rects_count, stats);
      render_area.y += row_length;
      render_area.height -= row_length;
    }

    start = end;
  }
}

TMV_API TMV_INLINE void tmv_squarify(
    tmv_rect area,    /* The area on which the squarified treemap should be aligned */
    tmv_item *items,  /* The descending by weight sorted treemap items*/
    int items_count,  /* The number of items */
    tmv_rect *rects,  /* The output rects that have been computed */
    int *rects_count, /* The output number of rects computed */
    tmv_stats *stats  /* The output stats/metrics */
)
{
  int i;
  int j;

  if (items_count == 0)
  {
    return;
  }

  /* Lay out the current level */
  tmv_squarify_current(area, items, items_count, rects, rects_count, stats);

  /* For each item, recurse into children if any */
  for (i = *rects_count - items_count, j = 0; j < items_count; ++j, ++i)
  {
    tmv_item *item = &items[j];

    if (item->children_count > 0)
    {
      tmv_rect parent_rect = rects[i];

      tmv_squarify(
          parent_rect,
          item->children,
          item->children_count,
          rects,
          rects_count,
          stats);
    }
  }
}

/* ########################################################## */
/* # Binary En-/Decoding of tmv data                          */
/* ########################################################## */
#define TMV_BINARY_SIZE_MAGIC 4
#define TMV_BINARY_VERSION 1
#define TMV_BINARY_SIZE_VERSION 4
#define TMV_BINARY_SIZE_COUNTS 28
#define TMV_BINARY_SIZE_HEADER (TMV_BINARY_SIZE_MAGIC + TMV_BINARY_SIZE_VERSION + TMV_BINARY_SIZE_COUNTS)

TMV_API TMV_INLINE void *tmv_binary_memcpy(void *dest, void *src, unsigned long count)
{
  char *dest8 = (char *)dest;
  const char *src8 = (char *)src;
  while (count--)
  {
    *dest8++ = *src8++;
  }

  return dest;
}

TMV_API TMV_INLINE void tmv_binary_encode(
    unsigned char *out_binary,         /* Output buffer for executable */
    unsigned long out_binary_capacity, /* Capacity of output buffer */
    unsigned long *out_binary_size,    /* Actual size of output binary buffer*/
    tmv_rect area,                     /* The area on which the squarified treemap should be aligned */
    tmv_item *items,                   /* The descending by weight sorted treemap items*/
    int items_count,                   /* The number of items */
    int items_user_data_size,          /* Size of user_data in bytes per item */
    tmv_rect *rects,                   /* The output rects that have been computed */
    int *rects_count,                  /* The output number of rects computed */
    tmv_stats *stats                   /* The output stats/metrics */
)
{
  unsigned char *ptr = out_binary;
  unsigned char *end = out_binary + out_binary_capacity;

  unsigned long size_struct_area = sizeof(area);
  unsigned long size_struct_stats = sizeof(*stats);
  unsigned long size_struct_items = (unsigned long)tmv_total_items(items, items_count) * ((unsigned long)sizeof(*items) + (unsigned long)items_user_data_size);
  unsigned long size_struct_rects = (unsigned long)*rects_count * (unsigned long)sizeof(*rects);

  if (end - ptr < 5)
  {
    return;
  }

  /* 4 byte magic */
  ptr[0] = 'T';
  ptr[1] = 'M';
  ptr[2] = 'V';
  ptr[3] = '\0';

  /* 1 byte version + 3 byte padding */
  ptr[4] = TMV_BINARY_VERSION;
  ptr[5] = 0;
  ptr[6] = 0;
  ptr[7] = 0;

  ptr += TMV_BINARY_SIZE_MAGIC + TMV_BINARY_SIZE_VERSION;

  /* 28 bytes count and struct metrics */
  tmv_binary_memcpy(ptr, &size_struct_area, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &size_struct_stats, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &size_struct_items, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &size_struct_rects, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &items_count, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &items_user_data_size, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, rects_count, 4);
  ptr += 4;

  /* Write the tmv data */
  tmv_binary_memcpy(ptr, &area, size_struct_area);
  ptr += size_struct_area;
  tmv_binary_memcpy(ptr, stats, size_struct_stats);
  ptr += size_struct_stats;
  tmv_binary_memcpy(ptr, items, size_struct_items);
  ptr += size_struct_items;
  tmv_binary_memcpy(ptr, rects, size_struct_rects);
  ptr += size_struct_rects;

  *out_binary_size = TMV_BINARY_SIZE_HEADER + size_struct_area + size_struct_stats + size_struct_items + size_struct_rects;
}

TMV_API TMV_INLINE unsigned long tmv_binary_read_ul(unsigned char *ptr)
{
  return ((unsigned long)ptr[0]) |
         ((unsigned long)ptr[1] << 8) |
         ((unsigned long)ptr[2] << 16) |
         ((unsigned long)ptr[3] << 24);
}

TMV_API TMV_INLINE void tmv_binary_decode(
    unsigned char *in_binary,     /* Output buffer for executable */
    unsigned long in_binary_size, /* Actual size of output binary buffer*/
    tmv_rect *area,               /* The area on which the squarified treemap should be aligned */
    tmv_item *items,              /* The descending by weight sorted treemap items*/
    int *items_count,             /* The number of items */
    int *items_user_data_size,    /* Size of user_data in bytes per item */
    tmv_rect *rects,              /* The output rects that have been computed */
    int *rects_count,             /* The output number of rects computed */
    tmv_stats *stats              /* The output stats/metrics */
)
{
  unsigned char *binary_ptr;

  unsigned long size_struct_area;
  unsigned long size_struct_stats;
  unsigned long size_struct_items;
  unsigned long size_struct_rects;

  if (in_binary_size < TMV_BINARY_SIZE_HEADER)
  {
    /* no valid tmv binary */
    return;
  }

  if (in_binary[0] != 'T' || in_binary[1] != 'M' || in_binary[2] != 'V' || in_binary[3] != '\0')
  {
    /* no right magic */
    return;
  }
  if (in_binary[4] != TMV_BINARY_VERSION)
  {
    /* no right version */
    return;
  }

  if (in_binary[5] != 0 || in_binary[6] != 0 || in_binary[7] != 0)
  {
    /* no right padding */
    return;
  }

  binary_ptr = in_binary + (TMV_BINARY_SIZE_MAGIC + TMV_BINARY_SIZE_VERSION);

  /* area size */
  size_struct_area = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  /* stats size */
  size_struct_stats = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  /* items size */
  size_struct_items = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  /* rects size */
  size_struct_rects = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  if (in_binary_size < TMV_BINARY_SIZE_HEADER + size_struct_area + size_struct_stats + size_struct_items + size_struct_rects)
  {
    /* no space for data */
    return;
  }

  *items_count = (int)tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  *items_user_data_size = (int)tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  *rects_count = (int)tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  area = (tmv_rect *)binary_ptr;
  binary_ptr += size_struct_area;

  stats = (tmv_stats *)binary_ptr;
  binary_ptr += size_struct_stats;

  items = (tmv_item *)binary_ptr;
  binary_ptr += size_struct_items;

  rects = (tmv_rect *)binary_ptr;

  (void)area;
  (void)stats;
  (void)items;
  (void)items_count;
  (void)items_user_data_size;
  (void)rects;
  (void)rects_count;
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
