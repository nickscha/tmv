/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../tmv.h"

#include "test.h" /* Simple Testing framework */

TMV_API TMV_INLINE double tmv_fabs(double x)
{
  return (x < 0.0) ? -x : x;
}

#define TMV_MAX_RECTS 1024
#define TMV_ASSERT_DBL_EQ(a, b) assert(tmv_fabs((a) - (b)) < 1e-6)

void tmv_test_print_rects(tmv_rect *rects, unsigned long rect_count)
{
  unsigned long i;
  for (i = 0; i < rect_count; ++i)
  {
    tmv_rect rect = rects[i];
    printf("id: %5lu, x: %5.2f, y: %5.2f, w: %5.2f, h: %5.2f\n", rect.id, rect.x, rect.y, rect.width, rect.height);
  }
}

void tmv_test_simple_sort(void)
{
  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 100, 100};

  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item items[] = {
      {1, 10.0, 0, 0, 0},
      {2, 3.0, 0, 0, 0},
      {3, 20.0, 0, 0, 0},
      {4, 1.0, 0, 0, 0}};

  /* The unified tmv model */
  tmv_model model = {0};
  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  assert(TMV_ARRAY_SIZE(items) == 4);

  tmv_squarify(&model, area);

  assert(model.rects_count == 4);

  /* Items are sorted by weight afterwards */
  assert(model.items[0].id == 3);
  assert(model.items[1].id == 1);
  assert(model.items[2].id == 2);
  assert(model.items[3].id == 4);

  tmv_test_print_rects(model.rects, model.rects_count);

  /* Rects are sorted by weight afterwards */
  assert(model.rects[0].id == 3);
  assert(model.rects[1].id == 1);
  assert(model.rects[2].id == 2);
  assert(model.rects[3].id == 4);
}

void tmv_test_simple_layout(void)
{
  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 100, 100};

  /* A buffer to store the computed rects */
  tmv_rect rects[TMV_MAX_RECTS];

  /*
     Expected squarified treemap output if width = 100 and height = 100:

     id: 1, x:  0, y:  0, width: 50, height: 50
     id: 2, x:  0, y: 50, width: 50, height: 50
     id: 3, x: 50, y:  0, width: 50, height: 50
     id: 4, x: 50, y: 50, width: 50, height: 50

  */
  tmv_item items[] = {
      {1, 10.0, 0, 0, 0},
      {2, 10.0, 0, 0, 0},
      {3, 10.0, 0, 0, 0},
      {4, 10.0, 0, 0, 0}};

  /* The unified tmv model */
  tmv_model model = {0};
  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  assert(TMV_ARRAY_SIZE(items) == 4);

  tmv_squarify(&model, area);
  assert(model.rects_count == 4);

  tmv_test_print_rects(model.rects, model.rects_count);

  assert(model.rects[0].id == 1 && model.rects[0].x == 0 && model.rects[0].y == 0 && model.rects[0].width == 50 && model.rects[0].height == 50);
  assert(model.rects[1].id == 2 && model.rects[1].x == 0 && model.rects[1].y == 50 && model.rects[1].width == 50 && model.rects[1].height == 50);
  assert(model.rects[2].id == 3 && model.rects[2].x == 50 && model.rects[2].y == 0 && model.rects[2].width == 50 && model.rects[2].height == 50);
  assert(model.rects[3].id == 4 && model.rects[3].x == 50 && model.rects[3].y == 50 && model.rects[3].width == 50 && model.rects[3].height == 50);
}

void tmv_test_simple_recursive_layout(void)
{
  tmv_item *found;

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 100, 100};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];

  /*
   Expected squarified treemap output if width = 100 and height = 100:

   id: 1, x:  0, y:  0, width: 50, height: 50
   id: 2, x:  0, y: 50, width: 50, height: 50
   id: 3, x: 50, y:  0, width: 50, height: 50
   id: 4, x: 50, y: 50, width: 50, height: 50
   id: 5, x:  0, y:  0, width: 25, height: 25  <- child1
   id: 6, x:  0, y: 25, width: 25, height: 25  <- child2
   id: 7, x: 25, y:  0, width: 25, height: 25  <- child3
   id: 8, x: 25, y: 25, width: 25, height: 25  <- child4
  */
  tmv_item child1 = {5, 2.5, 0, 0, 0};
  tmv_item child2 = {6, 2.5, 0, 0, 0};
  tmv_item child3 = {7, 2.5, 0, 0, 0};
  tmv_item child4 = {8, 2.5, 0, 0, 0};
  tmv_item children[4];

  tmv_item items[] = {
      {1, 10.0, 0, 0, 4},
      {2, 10.0, 0, 0, 0},
      {3, 10.0, 0, 0, 0},
      {4, 10.0, 0, 0, 0}};

  /* The unified tmv model */
  tmv_model model = {0};

  children[0] = child1;
  children[1] = child2;
  children[2] = child3;
  children[3] = child4;

  items[0].children = children;

  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model, /* The unified tmv_model contain all data */
      area    /* The render area size for treemap       */
  );

  /* Afterwards you can iterate through the item.rects with item.rects_count */

  assert(model.rects_count == 8);

  tmv_test_print_rects(model.rects, model.rects_count);

  assert(model.rects[0].id == 1 && model.rects[0].x == 0 && model.rects[0].y == 0 && model.rects[0].width == 50 && model.rects[0].height == 50);
  assert(model.rects[1].id == 2 && model.rects[1].x == 0 && model.rects[1].y == 50 && model.rects[1].width == 50 && model.rects[1].height == 50);
  assert(model.rects[2].id == 3 && model.rects[2].x == 50 && model.rects[2].y == 0 && model.rects[2].width == 50 && model.rects[2].height == 50);
  assert(model.rects[3].id == 4 && model.rects[3].x == 50 && model.rects[3].y == 50 && model.rects[3].width == 50 && model.rects[3].height == 50);

  /* Children equally spaced in 50x50 area from parent id == 1 */
  assert(model.rects[4].id == 5 && model.rects[4].x == 0 && model.rects[4].y == 0 && model.rects[4].width == 25 && model.rects[4].height == 25);
  assert(model.rects[5].id == 6 && model.rects[5].x == 0 && model.rects[5].y == 25 && model.rects[5].width == 25 && model.rects[5].height == 25);
  assert(model.rects[6].id == 7 && model.rects[6].x == 25 && model.rects[6].y == 0 && model.rects[6].width == 25 && model.rects[6].height == 25);
  assert(model.rects[7].id == 8 && model.rects[7].x == 25 && model.rects[7].y == 25 && model.rects[7].width == 25 && model.rects[7].height == 25);

  found = tmv_find_item_by_id(items, TMV_ARRAY_SIZE(items), 3);
  assert(found->id == 3);
  TMV_ASSERT_DBL_EQ(found->weight, 10.0);
}

void tmv_test_simple_more_items(void)
{
  unsigned long i;

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 100, 100};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];

#define TMVTSMI_ITEMS 625 /* 25*25 equal weighted items on a 100x100 grid */
  tmv_item items[TMVTSMI_ITEMS];

  /* The unified tmv model */
  tmv_model model = {0};
  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  for (i = 0; i < model.items_count; ++i)
  {
    tmv_item item = {0};
    item.id = i;
    item.weight = 1.0;
    items[i] = item;
  }

  /* Build squarified recursive treemap view */
  tmv_squarify(&model, area);

  assert(model.rects_count == TMVTSMI_ITEMS);

  for (i = 0; i < model.rects_count; ++i)
  {
    tmv_rect rect = rects[i];
    if (i % 100 == 0)
    {
      TMV_ASSERT_DBL_EQ(rect.width + rect.height, 8.0);
    }
  }
}

void tmv_test_binary_encode(void)
{
#define BINARY_BUFFER_CAPACITY 1024
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  unsigned long size_struct_items;
  unsigned long size_struct_rects;

  unsigned char *binary_ptr;
  tmv_rect *binary_area;
  tmv_stats *binary_stats;
  tmv_item *binary_items;
  tmv_rect *binary_rects;

  unsigned long i;

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {99, 0, 0, 100, 100};

  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item child1 = {5, 2.5, 0, 0, 0};
  tmv_item child2 = {6, 2.5, 0, 0, 0};
  tmv_item child3 = {7, 2.5, 0, 0, 0};
  tmv_item child4 = {8, 2.5, 0, 0, 0};
  tmv_item children[4];

  tmv_item items[] = {
      {1, 10.0, 0, 0, 4},
      {2, 10.0, 0, 0, 0},
      {3, 10.0, 0, 0, 0},
      {4, 10.0, 0, 0, 0}};

  tmv_model model = {0};

  children[0] = child1;
  children[1] = child2;
  children[2] = child3;
  children[3] = child4;

  items[0].children = children;

  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  assert(model.rects_count == 8);
  assert(model.rects_count == tmv_total_items(model.items, model.items_count));

  /* ########################################################## */
  /* # Encoding to binary                                       */
  /* ########################################################## */
  tmv_binary_encode(
      binary_buffer,
      BINARY_BUFFER_CAPACITY,
      &binary_buffer_size,
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  /* ########################################################## */
  /* # Decoding and Verifying binary                            */
  /* ########################################################## */
  printf("[bin] binary_buffer_size: %lu\n", binary_buffer_size);

  size_struct_items = tmv_total_items(items, TMV_ARRAY_SIZE(items)) * ((unsigned long)sizeof(*items) + model.items_user_data_size);
  size_struct_rects = model.rects_count * (unsigned long)sizeof(*rects);

  printf("[bin]  size_struct_items: %lu\n", size_struct_items);
  printf("[bin]  size_struct_rects: %lu\n", size_struct_rects);

  /* Test magic */
  assert(binary_buffer[0] == 'T');
  assert(binary_buffer[1] == 'M');
  assert(binary_buffer[2] == 'V');
  assert(binary_buffer[3] == '\0');

  /* Test version */
  assert(binary_buffer[4] == TMV_BINARY_VERSION);
  assert(binary_buffer[5] == 0);
  assert(binary_buffer[6] == 0);
  assert(binary_buffer[7] == 0);

  /* Test counts */
  binary_ptr = binary_buffer + (TMV_BINARY_SIZE_MAGIC + TMV_BINARY_SIZE_VERSION);

  assert(tmv_binary_read_ul(binary_ptr) == (unsigned long)sizeof(area));
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == (unsigned long)sizeof(model.stats));
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == size_struct_items);
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == size_struct_rects);
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == model.items_count);
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == model.items_user_data_size);
  binary_ptr += 4;
  assert(tmv_binary_read_ul(binary_ptr) == model.rects_count);
  binary_ptr += 4;

  /* check area */
  binary_area = (tmv_rect *)binary_ptr;
  assert(binary_area->id == 99);
  TMV_ASSERT_DBL_EQ(binary_area->x, 0);
  TMV_ASSERT_DBL_EQ(binary_area->y, 0);
  TMV_ASSERT_DBL_EQ(binary_area->width, 100.0);
  TMV_ASSERT_DBL_EQ(binary_area->height, 100.0);
  binary_ptr += (unsigned long)sizeof(area);

  /* check stats */
  binary_stats = (tmv_stats *)binary_ptr;
  assert(binary_stats->weigth_min == model.stats.weigth_min);
  assert(binary_stats->weigth_max == model.stats.weigth_max);
  assert(binary_stats->weigth_sum == model.stats.weigth_sum);
  assert(binary_stats->count == model.stats.count);
  binary_ptr += (unsigned long)sizeof(model.stats);

  /* check items */
  binary_items = (tmv_item *)binary_ptr;
  for (i = 0; i < model.items_count; ++i)
  {
    assert(binary_items[i].id == model.items[i].id);
    TMV_ASSERT_DBL_EQ(binary_items[i].weight, model.items[i].weight);
    assert(binary_items[i].children_count == model.items[i].children_count);
  }

  assert(binary_items[0].children_count == 4);

  /* Check items children s*/
  for (i = 0; i < binary_items[0].children_count; ++i)
  {
    assert(binary_items[0].children[i].id == model.items[0].children[i].id);
    TMV_ASSERT_DBL_EQ(binary_items[0].children[i].weight, model.items[0].children[i].weight);
    assert(binary_items[0].children[i].children_count == model.items[0].children[i].children_count);
  }

  binary_ptr += size_struct_items;

  /* check rects */
  binary_rects = (tmv_rect *)binary_ptr;
  for (i = 0; i < model.rects_count; ++i)
  {
    assert(binary_rects[i].id == model.rects[i].id);
    TMV_ASSERT_DBL_EQ(binary_rects[i].x, model.rects[i].x);
    TMV_ASSERT_DBL_EQ(binary_rects[i].y, model.rects[i].y);
    TMV_ASSERT_DBL_EQ(binary_rects[i].width, model.rects[i].width);
    TMV_ASSERT_DBL_EQ(binary_rects[i].height, model.rects[i].height);
  }
}

void tmv_test_binary_decode(void)
{
  unsigned long i;

  /* The encoded model data */
#define BINARY_BUFFER_CAPACITY 1024
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  /* The decoded model data */
  tmv_rect binary_area = {0};
  tmv_model binary_model = {0};

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {99, 0, 0, 100, 100};

  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item child1 = {5, 2.5, 0, 0, 0};
  tmv_item child2 = {6, 2.5, 0, 0, 0};
  tmv_item child3 = {7, 2.5, 0, 0, 0};
  tmv_item child4 = {8, 2.5, 0, 0, 0};
  tmv_item children[4];

  tmv_item items[] = {
      {1, 10.0, 0, 0, 4},
      {2, 10.0, 0, 0, 0},
      {3, 10.0, 0, 0, 0},
      {4, 10.0, 0, 0, 0}};

  tmv_model model = {0};

  children[0] = child1;
  children[1] = child2;
  children[2] = child3;
  children[3] = child4;

  items[0].children = children;

  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  assert(model.rects_count == 8);
  assert(model.rects_count == tmv_total_items(model.items, model.items_count));

  /* ########################################################## */
  /* # Encoding to binary                                       */
  /* ########################################################## */
  tmv_binary_encode(
      binary_buffer,
      BINARY_BUFFER_CAPACITY,
      &binary_buffer_size,
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  /* ########################################################## */
  /* # Decode from binary                                       */
  /* ########################################################## */
  tmv_binary_decode(
      binary_buffer,
      binary_buffer_size,
      &binary_model,
      &binary_area);

  /* Check area struct */
  assert(binary_area.id == area.id);
  TMV_ASSERT_DBL_EQ(binary_area.x, area.x);
  TMV_ASSERT_DBL_EQ(binary_area.y, area.y);
  TMV_ASSERT_DBL_EQ(binary_area.width, area.width);
  TMV_ASSERT_DBL_EQ(binary_area.height, area.height);

  /* Check model counts */
  assert(binary_model.items_count == model.items_count);
  assert(binary_model.items_user_data_size == model.items_user_data_size);
  assert(binary_model.rects_count == model.rects_count);

  /* Check model stats */
  assert(binary_model.stats.weigth_min == model.stats.weigth_min);
  assert(binary_model.stats.weigth_max == model.stats.weigth_max);
  assert(binary_model.stats.weigth_sum == model.stats.weigth_sum);
  assert(binary_model.stats.count == model.stats.count);

  /* Check model items */
  for (i = 0; i < model.items_count; ++i)
  {
    assert(binary_model.items[i].id == model.items[i].id);
    assert(binary_model.items[i].children_count == model.items[i].children_count);
    TMV_ASSERT_DBL_EQ(binary_model.items[i].weight, model.items[i].weight);
  }

  /* Check model childrens */
  for (i = 0; i < binary_model.items[0].children_count; ++i)
  {
    assert(binary_model.items[0].children[i].id == model.items[0].children[i].id);
    TMV_ASSERT_DBL_EQ(binary_model.items[0].children[i].weight, model.items[0].children[i].weight);
    assert(binary_model.items[0].children[i].children_count == model.items[0].children[i].children_count);
  }

  /* Check model rects */
  for (i = 0; i < model.rects_count; ++i)
  {
    assert(binary_model.rects[i].id == model.rects[i].id);
    TMV_ASSERT_DBL_EQ(binary_model.rects[i].x, model.rects[i].x);
    TMV_ASSERT_DBL_EQ(binary_model.rects[i].y, model.rects[i].y);
    TMV_ASSERT_DBL_EQ(binary_model.rects[i].width, model.rects[i].width);
    TMV_ASSERT_DBL_EQ(binary_model.rects[i].height, model.rects[i].height);
  }
}

int main(void)
{
  tmv_test_simple_sort();
  tmv_test_simple_layout();
  tmv_test_simple_recursive_layout();
  tmv_test_simple_more_items();
  tmv_test_binary_encode();
  tmv_test_binary_decode();

  return 0;
}

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
