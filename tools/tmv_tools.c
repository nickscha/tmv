/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../tmv.h"
#include "deps/vgg.h"
#include "deps/vgg_platform_write.h"

static vgg_color color_start = {144, 224, 239}; /* Start (light teal): 144, 224, 239 */
static vgg_color color_end = {255, 85, 0};      /*  End (dark orange): 255,  85,   0 */

#define TMV_MAX_RECTS 2048
#define VGG_MAX_BUFFER_SIZE 65536 /* SVG Buffer Size */

static void tmv_write_to_svg(char *filename, double area_width, double area_height, tmv_rect *rects, int rects_count, tmv_item *items, int items_count, double min_weight, double max_weight)
{
  int i;
  unsigned char vgg_buffer[VGG_MAX_BUFFER_SIZE];

  vgg_svg_writer w;
  w.buffer = vgg_buffer;
  w.capacity = VGG_MAX_BUFFER_SIZE;
  w.length = 0;

  vgg_svg_start(&w, "tmvsvg", area_width, area_height);

  for (i = 0; i < rects_count; ++i)
  {
    tmv_rect rect = rects[i];
    tmv_item *item = tmv_find_item_by_id(items, items_count, rect.id);

    vgg_rect r = {0};
    r.header.id = rect.id;
    r.header.type = VGG_TYPE_RECT;
    r.header.color_fill = vgg_color_map_linear(item->weight, min_weight, max_weight, color_start, color_end);
    r.x = rect.x;
    r.y = rect.y;
    r.width = rect.width;
    r.height = rect.height;

    vgg_svg_element_add(&w, (vgg_header *)&r);
  }

  vgg_svg_end(&w);
  vgg_platform_write(filename, w.buffer, (unsigned long)w.length);
}

#define TMV_LW_ITEMS 625 /* 25*25 equal weighted items on a 100x100 grid */
static void tmv_to_svg_linear_weights(void)
{

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 400, 400};

  tmv_stats stats = {0};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];
  int rect_count = 0;

  tmv_item items[TMV_LW_ITEMS];

  unsigned int i;

  for (i = 0; i < TMV_LW_ITEMS; ++i)
  {
    tmv_item item = {0};
    item.id = i;
    item.weight = TMV_LW_ITEMS - i;
    items[i] = item;
  }

  /* Build squarified recursive treemap view */
  tmv_squarify(
      area,                  /* The area on which the squarified treemap should be aligned */
      items,                 /* List of treemap items */
      TMV_ARRAY_SIZE(items), /* Size of top level items */
      rects,                 /* The output buffer for rectangular shapes computed */
      &rect_count,           /* The number of rectangular shapes computed */
      &stats                 /* The calculated statistic */
  );

  /* Write to SVG file */
  tmv_write_to_svg("tmv_to_svg_linear_weights.svg", area.width, area.height, rects, rect_count, items, tmv_total_items(items, TMV_ARRAY_SIZE(items)), stats.weigth_min, stats.weigth_max);
}

static void tmv_to_svg_nested(void)
{
  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 400, 400};

  tmv_stats stats = {0};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];
  int rect_count = 0;

  tmv_item child1 = {5, 2.5, 0, 0, 0};
  tmv_item child2 = {6, 2.5, 0, 0, 0};
  tmv_item child3 = {7, 2.5, 0, 0, 0};
  tmv_item child4 = {8, 2.5, 0, 0, 0};
  tmv_item children_linear[4];

  tmv_item child5 = {9, 5.0, 0, 0, 0};
  tmv_item child6 = {10, 2.5, 0, 0, 0};
  tmv_item child7 = {11, 1.25, 0, 0, 0};
  tmv_item child8 = {12, 1.25, 0, 0, 0};
  tmv_item children_weighted[4];

  tmv_item items[] = {
      {1, 20.0, 0, 0, 0},
      {2, 10.0, 0, 0, 0},
      {3, 5.0, 0, 0, 0},
      {4, 5.0, 0, 0, 0}};

  children_linear[0] = child1;
  children_linear[1] = child2;
  children_linear[2] = child3;
  children_linear[3] = child4;

  children_weighted[0] = child5;
  children_weighted[1] = child6;
  children_weighted[2] = child7;
  children_weighted[3] = child8;

  items[1].children = children_linear;
  items[1].children_count = 4;

  items[3].children = children_weighted;
  items[3].children_count = 4;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      area,        /* The area on which the squarified treemap should be aligned */
      items,       /* List of treemap items */
      4,           /* Size of top level items */
      rects,       /* The output buffer for rectangular shapes computed */
      &rect_count, /* The number of rectangular shapes computed */
      &stats       /* The calculated statistics */
  );

  /* Write to SVG file */
  tmv_write_to_svg("tmv_to_svg_nested.svg", area.width, area.height, rects, rect_count, items, tmv_total_items(items, TMV_ARRAY_SIZE(items)), stats.weigth_min, stats.weigth_max);
}

int main(void)
{
  tmv_to_svg_linear_weights();
  tmv_to_svg_nested();

  return 0;
}

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------
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
