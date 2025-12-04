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
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_INLINE __inline__
#elif defined(_MSC_VER)
#define TMV_INLINE __inline
#else
#define TMV_INLINE
#endif

#define TMV_API static

#define TMV_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define TMV_FIRST_VALID_PARENT_ID 0

typedef struct tmv_item
{

  /* User provided fields */
  long id;        /* The id of this item that is also used for the computed tmv_rect */
  long parent_id; /* The parent id of this item */
  double weight;  /* The weight of the item */

  /* Computed fields */
  unsigned long children_offset_index; /* The tmv_item index where the childrens are located */
  unsigned long children_count;        /* The number of childrens */

} tmv_item;

typedef struct tmv_rect
{
  long id;

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
  int items_sorted;                   /* Did the items have been sorted */
  unsigned long items_count;          /* The number of items */
  unsigned long items_user_data_size; /* The user_data size per item */
  unsigned long rects_count;          /* The output rects that have been computed */
  tmv_item *items;                    /* The descending by weight sorted treemap items*/
  tmv_rect *rects;                    /* The output rects that have been computed */

} tmv_model;

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

TMV_API TMV_INLINE tmv_item *tmv_find_item_by_id(tmv_item *items, unsigned long count, long id)
{
  unsigned long i;

  for (i = 0; i < count; ++i)
  {
    if (items[i].id == id)
    {
      return &items[i];
    }
  }

  return 0;
}

TMV_API TMV_INLINE tmv_rect *tmv_find_rect_by_id(tmv_rect *rects, unsigned long count, long id)
{
  unsigned long i;
  for (i = 0; i < count; ++i)
  {
    if (rects[i].id == id)
    {
      return &rects[i];
    }
  }
  return 0;
}

TMV_API TMV_INLINE void tmv_items_depth_sort_offset(tmv_item *items, unsigned long count)
{
  unsigned long i, j;
  int changed;
  unsigned long iteration;

  /* (1) Compute depths (iterative) */
  for (iteration = 0; iteration < count; ++iteration)
  {
    changed = 0;
    for (i = 0; i < count; i++)
    {
      tmv_item *item = &items[i];
      if (item->parent_id < TMV_FIRST_VALID_PARENT_ID)
      {
        if (item->children_offset_index != 0)
        {
          item->children_offset_index = 0;
          changed = 1;
        }
      }
      else
      {
        for (j = 0; j < count; ++j)
        {
          if (items[j].id == item->parent_id)
          {
            unsigned long new_depth = items[j].children_offset_index + 1;
            if (item->children_offset_index != new_depth)
            {
              item->children_offset_index = new_depth;
              changed = 1;
            }
            break;
          }
        }
      }
    }
    if (!changed)
    {
      break;
    }
  }

  /* 2) Stable insertion sort by depth (asc), parent_id (asc), weight (desc) */
  for (i = 1; i < count; ++i)
  {
    tmv_item key = items[i];
    j = i;

    while (j > 0)
    {
      tmv_item *prev = &items[j - 1];

      int depth_cmp = (int)prev->children_offset_index - (int)key.children_offset_index;
      int parent_cmp = (prev->parent_id < key.parent_id) ? -1 : (prev->parent_id > key.parent_id) ? 1
                                                                                                  : 0;
      int weight_cmp = (prev->weight < key.weight) ? 1 : (prev->weight > key.weight) ? -1
                                                                                     : 0;

      int should_swap = 0;

      if (depth_cmp > 0)
        should_swap = 1;
      else if (depth_cmp == 0)
      {
        if (parent_cmp > 0)
          should_swap = 1;
        else if (parent_cmp == 0 && weight_cmp > 0)
          should_swap = 1;
      }

      if (!should_swap)
        break;

      items[j] = items[j - 1];
      j--;
    }

    items[j] = key;
  }

  /* (3) Compute children offsets & counts in one pass */
  for (i = 0; i < count; ++i)
  {
    long pid = items[i].id;
    unsigned long offset = 0;
    unsigned long ccount = 0;

    for (j = i + 1; j < count; ++j)
    {
      if (items[j].parent_id == pid)
      {
        if (ccount == 0)
        {
          offset = j;
        }
        ccount++;
      }
      else if (ccount > 0)
      {
        break;
      }
    }
    items[i].children_offset_index = (ccount > 0) ? offset : 0;
    items[i].children_count = ccount;
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
    tmv_item row_item = row_items[i];

    double item_area = row_item.weight * scale;
    double w, h;

    /* Collect statistics */
    if (row_item.children_count == 0)
    {
      double weight = row_item.weight;

      if (model->stats.weigth_min < 0.0 || weight < model->stats.weigth_min)
      {
        model->stats.weigth_min = weight;
      }

      if (model->stats.weigth_max < 0.0 || weight > model->stats.weigth_max)
      {
        model->stats.weigth_max = weight;
      }

      model->stats.weigth_sum += weight;
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

    model->rects[model->rects_count].id = row_item.id;
    model->rects_count++;
  }
}

TMV_API TMV_INLINE void tmv_squarify_current(
    tmv_model *model,
    tmv_rect render_area /* The area on which the squarified treemap should be aligned */
)
{
  tmv_item *items = model->items;
  unsigned long items_count = model->items_count;

  unsigned long start = 0;
  double total_weight = tmv_total_weight(items, items_count);
  double area = render_area.width * render_area.height;
  double scale = (total_weight > 0.0) ? (area / total_weight) : 0.0;

  int horizontal = (render_area.width >= render_area.height);
  double side = horizontal ? render_area.height : render_area.width;

  while (start < items_count)
  {
    unsigned long end = start;
    double row_weight = 0.0;
    double worst = 1e9;
    unsigned long i;

    double row_length;
    unsigned long row_count;

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

    if (horizontal)
    {
      tmv_rect row_area = render_area;
      row_area.width = row_length;

      tmv_layout_row(model, row_area, &items[start], row_count);
      render_area.x += row_length;
      render_area.width -= row_length;
    }
    else
    {
      tmv_rect row_area = render_area;
      row_area.height = row_length;

      tmv_layout_row(model, row_area, &items[start], row_count);
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
  unsigned long i = 0;
  unsigned long root_start = 0;
  unsigned long root_count = 0;

  if (model->items_count == 0)
  {
    return;
  }

  if (!model->items_sorted)
  {
    model->stats.weigth_min = -1.0;
    model->stats.weigth_max = -1.0;
    model->stats.weigth_sum = 0.0;
    model->stats.count = 0;
    model->rects_count = 0;

    tmv_items_depth_sort_offset(model->items, model->items_count);

    model->items_sorted = 1;
  }

  /* Count number of root items */
  while ((root_start + root_count) < model->items_count &&
         model->items[root_start + root_count].parent_id < TMV_FIRST_VALID_PARENT_ID)
  {
    ++root_count;
  }

  /* Layout only root-level items at first */
  if (root_count > 0)
  {
    tmv_model root_model = *model;
    root_model.items = &model->items[root_start];
    root_model.items_count = root_count;

    tmv_squarify_current(&root_model, area);

    model->rects_count = root_model.rects_count;
    model->stats = root_model.stats;
  }

  /* Layout children for each node (already depth-sorted) */
  for (i = 0; i < model->items_count; ++i)
  {
    tmv_item *item = &model->items[i];

    if (item->children_count > 0)
    {
      tmv_model child_model;

      /* Find parent rect */
      tmv_rect *parent_rect = tmv_find_rect_by_id(model->rects, model->rects_count, item->id);

      if (!parent_rect)
      {
        continue;
      }

      /* Setup child model view */
      child_model = *model;
      child_model.items = &model->items[item->children_offset_index];
      child_model.items_count = item->children_count;

      /* Layout children directly in shared rect buffer */
      tmv_squarify_current(&child_model, *parent_rect);

      /* Update parent model state */
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

TMV_API TMV_INLINE void tmv_binary_encode(
    unsigned char *out_binary,         /* Output buffer for executable */
    unsigned long out_binary_capacity, /* Capacity of output buffer */
    unsigned long *out_binary_size,    /* Actual size of output binary buffer*/
    tmv_model *model,                  /* The tmv data */
    tmv_rect area                      /* The area on which the squarified treemap should be aligned */
)
{
  unsigned char *ptr = out_binary;

  unsigned long size_struct_area = sizeof(tmv_rect);
  unsigned long size_struct_stats = sizeof(tmv_stats);
  unsigned long size_struct_item = sizeof(tmv_item);
  unsigned long size_struct_rect = sizeof(tmv_rect);

  unsigned long size_items = model->items_count * (size_struct_item + model->items_user_data_size);
  unsigned long size_rects = model->rects_count * size_struct_rect;

  unsigned long size_total = TMV_BINARY_SIZE_HEADER + size_struct_area + size_struct_stats + size_items + size_rects;

  if (out_binary_capacity < size_total)
  {
    /* Binary buffer size cannot fit the tmv data */
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
  tmv_binary_memcpy(ptr, &size_struct_item, 4);
  ptr += 4;
  tmv_binary_memcpy(ptr, &size_struct_rect, 4);
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
  tmv_binary_memcpy(ptr, model->items, size_items);
  ptr += size_items;
  tmv_binary_memcpy(ptr, model->rects, size_rects);
  ptr += size_rects;

  *out_binary_size = size_total;
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
    tmv_model *model,             /* The tmv data model */
    tmv_rect *area                /* The area on which the squarified treemap should be aligned */
)
{
  unsigned char *binary_ptr;

  unsigned long size_struct_area;
  unsigned long size_struct_stats;
  unsigned long size_struct_item;
  unsigned long size_struct_rect;

  unsigned long size_items;
  unsigned long size_rects;

  unsigned long size_total;

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
  size_struct_item = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  /* rects size */
  size_struct_rect = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  model->items_count = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  model->items_user_data_size = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  model->rects_count = tmv_binary_read_ul(binary_ptr);
  binary_ptr += 4;

  /* total items size */
  size_items = model->items_count * (size_struct_item + model->items_user_data_size);
  size_rects = model->rects_count * size_struct_rect;

  size_total = TMV_BINARY_SIZE_HEADER + size_struct_area + size_struct_stats + size_items + size_rects;

  if (in_binary_size < size_total)
  {
    /* no space for data */
    return;
  }

  *area = *(tmv_rect *)binary_ptr;
  binary_ptr += size_struct_area;

  model->stats = *(tmv_stats *)binary_ptr;
  binary_ptr += size_struct_stats;

  model->items = (tmv_item *)binary_ptr;
  binary_ptr += size_items;

  model->rects = (tmv_rect *)binary_ptr;
  binary_ptr += size_rects;
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
