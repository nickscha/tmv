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

void tmv_test_print_rects(tmv_treemap_rect *rects, int rect_count)
{
  int i;
  for (i = 0; i < rect_count; ++i)
  {
    tmv_treemap_rect rect = rects[i];
    printf("id: %5i, x: %5.2f, y: %5.2f, w: %5.2f, h: %5.2f\n", rect.id, rect.x, rect.y, rect.width, rect.height);
  }
}

void tmv_test_simple_layout(void)
{
  tmv_treemap_rect rects[TMV_MAX_RECTS];
  int rect_count = 0;

  /*
     Expected squarified treemap output if width = 100 and height = 100:

     id: 1, x:  0, y:  0, width: 50, height: 50
     id: 2, x:  0, y: 50, width: 50, height: 50
     id: 3, x: 50, y:  0, width: 50, height: 50
     id: 4, x: 50, y: 50, width: 50, height: 50

  */
  tmv_treemap_item items[] = {
      {1, 10.0, 0, 0},
      {2, 10.0, 0, 0},
      {3, 10.0, 0, 0},
      {4, 10.0, 0, 0}};

  assert(TMV_ARRAY_SIZE(items) == 4);

  tmv_squarify(items, TMV_ARRAY_SIZE(items), 0, 0, 100, 100, rects, &rect_count);
  assert(rect_count == 4);

  tmv_test_print_rects(rects, rect_count);

  assert(rects[0].id == 1 && rects[0].x == 0 && rects[0].y == 0 && rects[0].width == 50 && rects[0].height == 50);
  assert(rects[1].id == 2 && rects[1].x == 0 && rects[1].y == 50 && rects[1].width == 50 && rects[1].height == 50);
  assert(rects[2].id == 3 && rects[2].x == 50 && rects[2].y == 0 && rects[2].width == 50 && rects[2].height == 50);
  assert(rects[3].id == 4 && rects[3].x == 50 && rects[3].y == 50 && rects[3].width == 50 && rects[3].height == 50);
}

void tmv_test_simple_recursive_layout(void)
{
  tmv_treemap_item *found;

  /* Define a output buffer for output rects */
  tmv_treemap_rect rects[TMV_MAX_RECTS];
  int rect_count = 0;

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
  tmv_treemap_item child1 = {5, 2.5, 0, 0};
  tmv_treemap_item child2 = {6, 2.5, 0, 0};
  tmv_treemap_item child3 = {7, 2.5, 0, 0};
  tmv_treemap_item child4 = {8, 2.5, 0, 0};
  tmv_treemap_item children[4];

  tmv_treemap_item items[] = {
      {1, 10.0, 0, 4},
      {2, 10.0, 0, 0},
      {3, 10.0, 0, 0},
      {4, 10.0, 0, 0}};

  children[0] = child1;
  children[1] = child2;
  children[2] = child3;
  children[3] = child4;

  items[0].children = children;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      items,                 /* List of treemap items */
      TMV_ARRAY_SIZE(items), /* Size of top level items */
      0, 0,                  /* Treemap view area start */
      100, 100,              /* Treemap view area width and height */
      rects,                 /* The output buffer for rectangular shapes computed */
      &rect_count            /* The number of rectangular shapes computed */
  );

  /* Afterwards you can iterate through the rects */

  assert(rect_count == 8);

  tmv_test_print_rects(rects, rect_count);

  assert(rects[0].id == 1 && rects[0].x == 0 && rects[0].y == 0 && rects[0].width == 50 && rects[0].height == 50);
  assert(rects[1].id == 2 && rects[1].x == 0 && rects[1].y == 50 && rects[1].width == 50 && rects[1].height == 50);
  assert(rects[2].id == 3 && rects[2].x == 50 && rects[2].y == 0 && rects[2].width == 50 && rects[2].height == 50);
  assert(rects[3].id == 4 && rects[3].x == 50 && rects[3].y == 50 && rects[3].width == 50 && rects[3].height == 50);

  /* Children equally spaced in 50x50 area from parent id == 1 */
  assert(rects[4].id == 5 && rects[4].x == 0 && rects[4].y == 0 && rects[4].width == 25 && rects[4].height == 25);
  assert(rects[5].id == 6 && rects[5].x == 0 && rects[5].y == 25 && rects[5].width == 25 && rects[5].height == 25);
  assert(rects[6].id == 7 && rects[6].x == 25 && rects[6].y == 0 && rects[6].width == 25 && rects[6].height == 25);
  assert(rects[7].id == 8 && rects[7].x == 25 && rects[7].y == 25 && rects[7].width == 25 && rects[7].height == 25);

  found = tmv_find_item_by_id(items, TMV_ARRAY_SIZE(items), 3);
  assert(found->id == 3);
  TMV_ASSERT_DBL_EQ(found->weight, 10.0);
}

void tmv_test_simple_more_items(void)
{
  /* Define a output buffer for output rects */
  tmv_treemap_rect rects[TMV_MAX_RECTS];
  int rect_count = 0;

#define TMVTSMI_ITEMS 625 /* 25*25 equal weighted items on a 100x100 grid */
  tmv_treemap_item items[TMVTSMI_ITEMS];

  int i;

  for (i = 0; i < TMVTSMI_ITEMS; ++i)
  {
    tmv_treemap_item item = {0};
    item.id = i;
    item.weight = 1.0;
    items[i] = item;
  }

  /* Build squarified recursive treemap view */
  tmv_squarify(
      items,                 /* List of treemap items */
      TMV_ARRAY_SIZE(items), /* Size of top level items */
      0, 0,                  /* Treemap view area start */
      100, 100,              /* Treemap view area width and height */
      rects,                 /* The output buffer for rectangular shapes computed */
      &rect_count            /* The number of rectangular shapes computed */
  );

  assert(rect_count == TMVTSMI_ITEMS);

  for (i = 0; i < rect_count; ++i)
  {
    tmv_treemap_rect rect = rects[i];
    TMV_ASSERT_DBL_EQ(rect.width + rect.height, 8.0);
  }
}

int main(void)
{
  tmv_test_simple_layout();
  tmv_test_simple_recursive_layout();
  tmv_test_simple_more_items();

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
