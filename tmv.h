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
  unsigned long id;

  double weight;

  /* Additional user data */
  void *user_data;

  /* Hierarchy */
  struct tmv_item *children;
  unsigned long children_count;

} tmv_item;

typedef struct tmv_rect
{
  unsigned long id;

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
  unsigned long count;

} tmv_stats;

typedef struct tmv_model
{
  tmv_stats stats;                    /* The calculated stats and metrics  */
  unsigned long items_count;          /* The number of items */
  unsigned long items_user_data_size; /* The user_data size per item */
  unsigned long rects_count;          /* The output rects that have been computed */
  tmv_item *items;                    /* The descending by weight sorted treemap items*/
  tmv_rect *rects;                    /* The output rects that have been computed */

} tmv_model;

TMV_API TMV_INLINE unsigned long tmv_total_items(tmv_item *items, unsigned long items_count)
{
  unsigned long total = 0;
  unsigned long i;

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

TMV_API TMV_INLINE double tmv_total_weight(tmv_item *items, unsigned long count)
{
  double sum = 0.0;
  unsigned long i;
  for (i = 0; i < count; ++i)
  {
    sum += items[i].weight;
  }
  return sum;
}

TMV_API TMV_INLINE tmv_item *tmv_find_item_by_id(tmv_item *items, unsigned long count, unsigned long id)
{
  unsigned long i;
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

TMV_API TMV_INLINE void tmv_insertion_sort_stable_desc(tmv_item *items, unsigned long count)
{
  unsigned long i;
  long j;
  for (i = 1; i < count; ++i)
  {
    tmv_item key = items[i];
    j = (long)(i - 1);

    /* Move elements with weight < key.weight one position forward */
    while (j >= 0 && items[j].weight < key.weight)
    {
      items[j + 1] = items[j];
      j--;
    }
    items[j + 1] = key;
  }
}

TMV_API TMV_INLINE void tmv_layout_row(
    tmv_model *model,
    tmv_rect row_area,
    tmv_item *row_items,
    unsigned long row_count)
{
  unsigned long i;
  double area = row_area.width * row_area.height;
  double total_weight = tmv_total_weight(row_items, row_count);
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  int horizontal = (row_area.width >= row_area.height);
  double offset = 0.0;

  for (i = 0; i < row_count; ++i)
  {
    double item_area = row_items[i].weight * scale;
    double w, h;

    /* Collect statistics */
    if (row_items[i].children_count == 0)
    {
      double w = row_items[i].weight;

      if (model->stats.weigth_min < 0.0 || w < model->stats.weigth_min)
      {
        model->stats.weigth_min = w;
      }

      if (model->stats.weigth_max < 0.0 || w > model->stats.weigth_max)
      {
        model->stats.weigth_max = w;
      }

      model->stats.weigth_sum += w;
      model->stats.count += 1;
    }

    /* Add rects */
    if (horizontal)
    {
      w = item_area / row_area.height;
      h = row_area.height;
      model->rects[model->rects_count].x = row_area.x + offset;
      model->rects[model->rects_count].y = row_area.y;
      model->rects[model->rects_count].width = w;
      model->rects[model->rects_count].height = h;
      offset += w;
    }
    else
    {
      w = row_area.width;
      h = item_area / row_area.width;
      model->rects[model->rects_count].x = row_area.x;
      model->rects[model->rects_count].y = row_area.y + offset;
      model->rects[model->rects_count].width = w;
      model->rects[model->rects_count].height = h;
      offset += h;
    }

    model->rects[model->rects_count].id = row_items[i].id;
    model->rects_count++;
  }
}

TMV_API TMV_INLINE void tmv_squarify_current(
    tmv_model *model,
    tmv_rect render_area /* The area on which the squarified treemap should be aligned */
)
{
  unsigned long start = 0;
  double total_weight = tmv_total_weight(model->items, model->items_count);
  double area = render_area.width * render_area.height;
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  tmv_insertion_sort_stable_desc(model->items, model->items_count);

  while (start < model->items_count)
  {
    unsigned long end = start;
    double row_weight = 0.0;
    double worst = 1e9;
    unsigned long i;

    double side = (render_area.width >= render_area.height) ? render_area.height : render_area.width;

    double row_length;
    unsigned long row_count;

    /* Try to add items[start..end] */
    while (end < model->items_count)
    {
      double max_w = -1e9;
      double min_w = 1e9;
      double w_scaled;

      double row_area;
      double r1;
      double r2;
      double new_worst;

      row_weight += model->items[end].weight;

      /* Compute scaled weights and track min/max */
      for (i = start; i <= end; ++i)
      {
        w_scaled = model->items[i].weight * scale;
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
        row_weight -= model->items[end].weight;
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

      tmv_layout_row(model, row_area, &model->items[start], row_count);
      render_area.x += row_length;
      render_area.width -= row_length;
    }
    else
    {
      tmv_rect row_area = render_area;
      row_area.height = row_length;

      tmv_layout_row(model, row_area, &model->items[start], row_count);
      render_area.y += row_length;
      render_area.height -= row_length;
    }

    start = end;
  }
}

TMV_API TMV_INLINE void tmv_squarify(
    tmv_model *model,
    tmv_rect area /* The area on which the squarified treemap should be aligned */
)
{
  unsigned long i;
  unsigned long j;

  if (model->items_count == 0)
  {
    return;
  }

  /* Lay out the current level */
  tmv_squarify_current(model, area);

  /* For each item, recurse into children if any */
  for (i = model->rects_count - model->items_count, j = 0; j < model->items_count; ++j, ++i)
  {
    tmv_item *item = &model->items[j];

    if (item->children_count > 0)
    {
      tmv_rect parent_rect = model->rects[i];
      tmv_model child_model;
      child_model.items = item->children;
      child_model.items_count = item->children_count;
      child_model.rects = model->rects;
      child_model.rects_count = model->rects_count;
      child_model.stats = model->stats;

      tmv_squarify(&child_model, parent_rect);

      /* Update parent's rect count with value from child */
      model->rects_count = child_model.rects_count;
      model->stats = child_model.stats;
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

/* Calculates the total size of the tmv_item struct data with its recursive level of children */
TMV_API TMV_INLINE unsigned long tmv_binary_items_size(tmv_item *item, unsigned long items_user_data_size)
{
  unsigned long size = 0;
  unsigned long i;

  size += sizeof(unsigned long); /* id */
  size += sizeof(double);        /* weight */
  size += sizeof(unsigned long); /* children_count */
  size += items_user_data_size;  /* user_data_size */

  for (i = 0; i < item->children_count; ++i)
  {
    size += tmv_binary_items_size(&item->children[i], items_user_data_size);
  }

  return size;
}

/* Flattens the tmv_item struct data with its recursive level of children */
TMV_API TMV_INLINE char *tmv_binary_encode_item(unsigned char *out_binary, tmv_item *item, unsigned long items_user_data_size)
{
  unsigned long i;

  *(unsigned long *)out_binary = item->id;
  out_binary += sizeof(unsigned long *);

  *(double *)out_binary = item->weight;
  out_binary += sizeof(double);

  *(unsigned long *)out_binary = item->children_count;
  out_binary += sizeof(unsigned long);

  /* Copy user_data */
  for (i = 0; i < items_user_data_size; ++i)
  {
    *out_binary++ = ((unsigned char *)item->user_data)[i];
  }

  for (i = 0; i < item->children_count; ++i)
  {
    out_binary = tmv_binary_encode_item(out_binary, &item->children[i], items_user_data_size);
  }

  return out_binary;
}

TMV_API TMV_INLINE void tmv_binary_encode(
    unsigned char *out_binary,         /* Output buffer for executable */
    unsigned long out_binary_capacity, /* Capacity of output buffer */
    unsigned long *out_binary_size,    /* Actual size of output binary buffer*/
    tmv_model *model,                  /* The tmv data */
    tmv_rect area                      /* The area on which the squarified treemap should be aligned */
)
{
  unsigned char *ptr = out_binary;
  unsigned char *end = out_binary + out_binary_capacity;

  unsigned long size_struct_area = sizeof(area);
  unsigned long size_struct_stats = sizeof(model->stats);
  unsigned long size_struct_items = tmv_total_items(model->items, model->items_count) * ((unsigned long)sizeof(*model->items) + model->items_user_data_size);
  unsigned long size_struct_rects = model->rects_count * (unsigned long)sizeof(*model->rects);

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
  tmv_binary_memcpy(ptr, &model->items_count, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &model->items_user_data_size, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &model->rects_count, 4);
  ptr += 4;

  /* Write the tmv data */
  tmv_binary_memcpy(ptr, &area, size_struct_area);
  ptr += size_struct_area;
  tmv_binary_memcpy(ptr, &model->stats, size_struct_stats);
  ptr += size_struct_stats;
  tmv_binary_memcpy(ptr, model->items, size_struct_items);
  ptr += size_struct_items;
  tmv_binary_memcpy(ptr, model->rects, size_struct_rects);
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
    tmv_model *model,
    tmv_rect *area /* The area on which the squarified treemap should be aligned */
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

  model->items_count = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  model->items_user_data_size = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  model->rects_count = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  *area = *(tmv_rect *)binary_ptr;
  binary_ptr += size_struct_area;

  model->stats = *(tmv_stats *)binary_ptr;
  binary_ptr += size_struct_stats;

  model->items = (tmv_item *)binary_ptr;
  binary_ptr += size_struct_items;

  model->rects = (tmv_rect *)binary_ptr;
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
