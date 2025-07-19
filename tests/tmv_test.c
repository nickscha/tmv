/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../tmv.h"

#include "test.h" /* Simple Testing framework */

#define TVM_TEST_EPSILON 1e-6
#define TMV_MAX_RECTS 1024

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
      {1, -1, 10.0, 0, 0},
      {2, -1, 3.0, 0, 0},
      {3, -1, 20.0, 0, 0},
      {4, -1, 1.0, 0, 0}};

  /* The unified tmv model */
  tmv_model model = {0};
  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  assert(TMV_ARRAY_SIZE(items) == 4);

  tmv_squarify(&model, area);

  /* Ensure that depending on compiler settings there is no weird pointer mismatching
   and the item fields (id, parent_id, weight) are the original ones after the squarification */
  assert(model.items[1].id == 1);
  assert(model.items[1].parent_id == -1);
  assert_equalsd(model.items[1].weight, 10.0, TVM_TEST_EPSILON);
  assert_equalsd(model.stats.weigth_min, 1.0, TVM_TEST_EPSILON);
  assert_equalsd(model.stats.weigth_max, 20.0, TVM_TEST_EPSILON);
  assert_equalsd(model.stats.weigth_sum, 34.0, TVM_TEST_EPSILON);

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
  tmv_item items[4] = {
      {1, -1, 10.0, 0, 0},
      {2, -1, 10.0, 0, 0},
      {3, -1, 10.0, 0, 0},
      {4, -1, 10.0, 0, 0}};

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
  tmv_item items[8] = {
      {1, -1, 10.0, 0, 0},
      {2, -1, 10.0, 0, 0},
      {3, -1, 10.0, 0, 0},
      {4, -1, 10.0, 0, 0},
      {5, 1, 2.5, 0, 0},
      {6, 1, 2.5, 0, 0},
      {7, 1, 2.5, 0, 0},
      {8, 1, 2.5, 0, 0},

  };

  /* The unified tmv model */
  tmv_model model = {0};

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
  assert_equalsd(found->weight, 10.0, TVM_TEST_EPSILON);
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

  for (i = 0; i < TMV_ARRAY_SIZE(items); ++i)
  {
    tmv_item item = {0};
    item.id = (long)i;
    item.parent_id = -1;
    item.weight = 1.0;
    items[i] = item;
  }

  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  /* Build squarified recursive treemap view */
  tmv_squarify(&model, area);

  assert(model.rects_count == TMVTSMI_ITEMS);

  for (i = 0; i < model.rects_count; ++i)
  {
    tmv_rect rect = rects[i];
    if (i % 100 == 0)
    {
      assert_equalsd(rect.width + rect.height, 8.0, TVM_TEST_EPSILON);
    }
  }
}

void tmv_test_flat_tree(void)
{
  unsigned long i;

  tmv_rect rects[TMV_MAX_RECTS];
  tmv_rect area = {0, 0.0, 0.0, 100.0, 100.0};
  tmv_model model = {0};

  /* Layout
     [0] p1
     [1] p2:
       [4] c1
       [5] c2
     [2] p3
     [3] p4
       [6] c3
         [8] cc1
         [9] cc2
       [7] c4
  */
  tmv_item item_p1 = {0, -1, 20.0, 0, 0};
  tmv_item item_p2 = {1, -1, 10.0, 0, 0};
  tmv_item item_p3 = {2, -1, 5.0, 0, 0};
  tmv_item item_p4 = {3, -1, 5.0, 0, 0};

  tmv_item item_c1 = {4, 1, 5.0, 0, 0};
  tmv_item item_c2 = {5, 1, 5.0, 0, 0};

  tmv_item item_c3 = {6, 3, 3.5, 0, 0};
  tmv_item item_c4 = {7, 3, 1.5, 0, 0};

  tmv_item item_cc1 = {8, 6, 1.75, 0, 0};
  tmv_item item_cc2 = {9, 6, 1.75, 0, 0};

  /* Unordered random sort */
  tmv_item items[10];
  items[0] = item_c1;
  items[1] = item_p3;
  items[2] = item_c2;
  items[3] = item_cc1;
  items[4] = item_p4;
  items[5] = item_p1;
  items[6] = item_p2;
  items[7] = item_c3;
  items[8] = item_cc2;
  items[9] = item_c4;

  assert(TMV_ARRAY_SIZE(items) == 10);

  /* For the flat tree */
  tmv_items_depth_sort_offset(items, TMV_ARRAY_SIZE(items));

  for (i = 0; i < TMV_ARRAY_SIZE(items); ++i)
  {
    printf("[%2li] id: %3lu, parent_id: %3li, weight: %10f, children_offset_index: %5lu, children_count: %5lu\n",
           i,
           items[i].id,
           items[i].parent_id,
           items[i].weight,
           items[i].children_offset_index,
           items[i].children_count);
  }

  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);
  model.rects = rects;

  tmv_squarify(&model, area);

  tmv_test_print_rects(model.rects, model.rects_count);
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

  tmv_item items[8] = {
      {1, -1, 10.0, 0, 0},
      {2, -1, 10.0, 0, 0},
      {3, -1, 10.0, 0, 0},
      {4, -1, 10.0, 0, 0},
      {5, 1, 2.5, 0, 0},
      {6, 1, 2.5, 0, 0},
      {7, 1, 2.5, 0, 0},
      {8, 1, 2.5, 0, 0}};

  tmv_model model = {0};

  model.rects = rects;
  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  assert(model.rects_count == 8);
  assert(model.rects_count == model.items_count);

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
  assert_equalsd(binary_area.x, area.x, TVM_TEST_EPSILON);
  assert_equalsd(binary_area.y, area.y, TVM_TEST_EPSILON);
  assert_equalsd(binary_area.width, area.width, TVM_TEST_EPSILON);
  assert_equalsd(binary_area.height, area.height, TVM_TEST_EPSILON);

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
    assert_equalsd(binary_model.items[i].weight, model.items[i].weight, TVM_TEST_EPSILON);
  }

  /* Check model rects */
  for (i = 0; i < model.rects_count; ++i)
  {
    assert(binary_model.rects[i].id == model.rects[i].id);
    assert_equalsd(binary_model.rects[i].x, model.rects[i].x, TVM_TEST_EPSILON);
    assert_equalsd(binary_model.rects[i].y, model.rects[i].y, TVM_TEST_EPSILON);
    assert_equalsd(binary_model.rects[i].width, model.rects[i].width, TVM_TEST_EPSILON);
    assert_equalsd(binary_model.rects[i].height, model.rects[i].height, TVM_TEST_EPSILON);
  }
}

int main(void)
{
  tmv_test_simple_sort();
  tmv_test_simple_layout();
  tmv_test_simple_recursive_layout();
  tmv_test_simple_more_items();
  tmv_test_flat_tree();
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
