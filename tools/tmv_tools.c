/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../tmv.h"             /* The Squarified Treemap algorythm   */
#include "../tmv_platform_io.h" /* Optional: Used for read/write file */
#include "deps/clp.h"           /* Command Line Parser                */
#include "deps/vgg.h"           /* SVG Graphics Generator             */

#include "../tests/test.h"

static vgg_color color_start = {144, 224, 239}; /* Start (light teal): 144, 224, 239 */
static vgg_color color_end = {255, 85, 0};      /*  End (dark orange): 255,  85,   0 */

#define TMV_MAX_RECTS 2048
#define VGG_MAX_BUFFER_SIZE 65536 /* SVG Buffer Size */

static void tmv_write_to_svg(char *filename, tmv_model *model, tmv_rect *area)
{
  unsigned long i;
  unsigned char vgg_buffer[VGG_MAX_BUFFER_SIZE];

  vgg_svg_writer w;
  w.buffer = vgg_buffer;
  w.capacity = VGG_MAX_BUFFER_SIZE;
  w.length = 0;

  vgg_svg_start(&w, "tmvsvg", area->width, area->height);

  for (i = 0; i < model->rects_count; ++i)
  {
    tmv_rect rect = model->rects[i];
    tmv_item *item = tmv_find_item_by_id(model->items, model->items_count, rect.id);

    vgg_rect r = {0};
    r.header.id = (unsigned long)rect.id;
    r.header.type = VGG_TYPE_RECT;
    r.header.color_fill = vgg_color_map_linear(item->weight, model->stats.weigth_min, model->stats.weigth_max, color_start, color_end);
    r.x = rect.x;
    r.y = rect.y;
    r.width = rect.width;
    r.height = rect.height;

    vgg_svg_element_add(&w, (vgg_header *)&r);
  }

  vgg_svg_end(&w);
  tmv_platform_write(filename, w.buffer, (unsigned long)w.length);
}

#define TMV_LW_ITEMS 625 /* 25*25 equal weighted items on a 100x100 grid */
static void tmv_to_svg_linear_weights(void)
{

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 400, 400};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item items[TMV_LW_ITEMS];

  tmv_model model = {0};

  long i;

  for (i = 0; i < TMV_LW_ITEMS; ++i)
  {
    tmv_item item = {0};
    item.id = i;
    item.parent_id = -1;
    item.weight = TMV_LW_ITEMS - i;
    items[i] = item;
  }

  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);
  model.rects = rects;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  /* Write to SVG file */
  tmv_write_to_svg("tmv_to_svg_linear_weights.svg", &model, &area);
}

static void tmv_to_svg_nested(void)
{
  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 400, 400};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item items[] = {
      {1, -1, 20.0, 0, 0},
      {2, -1, 10.0, 0, 0},
      {3, -1, 5.0, 0, 0},
      {4, -1, 5.0, 0, 0},
      {5, 2, 2.5, 0, 0},
      {6, 2, 2.5, 0, 0},
      {7, 2, 2.5, 0, 0},
      {8, 2, 2.5, 0, 0},
      {9, 4, 5.0, 0, 0},
      {10, 4, 2.5, 0, 0},
      {11, 4, 1.25, 0, 0},
      {12, 4, 1.25, 0, 0}};

  tmv_model model = {0};

  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);
  model.rects = rects;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

  /* Write to SVG file */
  tmv_write_to_svg("tmv_to_svg_nested.svg", &model, &area);
}

static void tmv_tools_binary_encode(void)
{
#define BINARY_BUFFER_CAPACITY 8192
  unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
  unsigned long binary_buffer_size = 0;

  /* The area on which the squarified treemap should be aligned */
  tmv_rect area = {0, 0, 0, 400, 400};

  /* Define a output buffer for output rects */
  tmv_rect rects[TMV_MAX_RECTS];

  tmv_item items[] = {
      {1, -1, 20.0, 0, 0},
      {2, -1, 10.0, 0, 0},
      {3, -1, 5.0, 0, 0},
      {4, -1, 5.0, 0, 0},
      {5, 2, 2.5, 0, 0},
      {6, 2, 2.5, 0, 0},
      {7, 2, 2.5, 0, 0},
      {8, 2, 2.5, 0, 0},
      {9, 4, 5.0, 0, 0},
      {10, 4, 2.5, 0, 0},
      {11, 4, 1.25, 0, 0},
      {12, 4, 1.25, 0, 0}};

  tmv_model model = {0};

  model.items = items;
  model.items_count = TMV_ARRAY_SIZE(items);
  model.rects = rects;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area /* The area on which the squarified treemap should be aligned */
  );

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

  /* Write to TMV file */
  assert(tmv_platform_write("tmv_tools_binary.tmv", binary_buffer, binary_buffer_size));
}

static void tmv_tools_binary_decode(void)
{
#define FILE_BUFFER_CAPACITY 8192
  unsigned char file_buffer[FILE_BUFFER_CAPACITY];
  unsigned long file_buffer_size = 0;

  tmv_rect area = {0};
  tmv_model model = {0};

  /* Read binary file */
  assert(tmv_platform_read("tmv_tools_binary.tmv", file_buffer, FILE_BUFFER_CAPACITY, &file_buffer_size));

  /* Decode binary file to model */
  tmv_binary_decode(file_buffer, file_buffer_size, &model, &area);

  assert(model.items_count == 12);
  assert(model.rects_count == 12);
  assert(model.items[1].children_count == 4);
  assert(model.items[3].children_count == 4);
}

#define TMV_TOOLS_FLAGS 3

void tmv_tools_tmv_to_svg(char *input_tmv_file, char *output_svg_file)
{
  unsigned long i;

#define TMV_TOOLS_FILE_BUFFER_CAPACITY 8192
  unsigned char file_buffer[TMV_TOOLS_FILE_BUFFER_CAPACITY];
  unsigned long file_buffer_size = 0;

  tmv_model model = {0};
  tmv_rect area = {0};

  tmv_platform_read(input_tmv_file, file_buffer, TMV_TOOLS_FILE_BUFFER_CAPACITY, &file_buffer_size);

  tmv_binary_decode(file_buffer, file_buffer_size, &model, &area);

  printf("[area]                    id: %10lu\n", area.id);
  printf("[area]                     x: %10f\n", area.x);
  printf("[area]                     y: %10f\n", area.y);
  printf("[area]                 width: %10f\n", area.width);
  printf("[area]                height: %10f\n", area.height);
  printf("\n");
  printf("[model]          items_count: %10lu\n", model.items_count);
  printf("[model] items_user_data_size: %10lu\n", model.items_user_data_size);
  printf("[model]          rects_count: %10lu\n", model.rects_count);
  printf("[model]           weigth_min: %10f\n", model.stats.weigth_min);
  printf("[model]           weigth_max: %10f\n", model.stats.weigth_max);
  printf("[model]           weigth_sum: %10f\n", model.stats.weigth_sum);
  printf("[model]                count: %10lu\n", model.stats.count);
  printf("\n");

  for (i = 0; i < model.items_count; ++i)
  {
    tmv_item item = model.items[i];

    printf(
        " [item][%2lu] id: %5lu, parent_id: %5li, weight: %10f, child_count: %5lu\n",
        i,
        item.id,
        item.parent_id,
        item.weight,
        item.children_count);
  }

  tmv_write_to_svg(output_svg_file, &model, &area);
}

int main(int argc, char **argv)
{
  clp_flag flags[TMV_TOOLS_FLAGS];
  char flag_command[32] = {0};
  char flag_input[32] = {0};
  char flag_output[32] = {0};

  flags[0].name = "cmd";
  flags[0].value = flag_command;
  flags[0].def_value = "tmv_to_svg";
  flags[0].maxlen = sizeof(flag_command);
  flags[0].type = FLAG_STRING;

  flags[1].name = "input";
  flags[1].value = flag_input;
  flags[1].def_value = "";
  flags[1].maxlen = sizeof(flag_input);
  flags[1].type = FLAG_STRING;

  flags[2].name = "output";
  flags[2].value = flag_output;
  flags[2].def_value = "";
  flags[2].maxlen = sizeof(flag_output);
  flags[2].type = FLAG_STRING;

  /* Parse the command line arguments */
  clp_process(flags, CLP_ARRAY_SIZE(flags), argv, argc);

  printf("[tmv_tools][cli]    cmd: %s\n", flag_command);
  printf("[tmv_tools][cli]  input: %s\n", flag_input);
  printf("[tmv_tools][cli] output: %s\n", flag_output);
  printf("\n");

  tmv_to_svg_linear_weights();
  tmv_to_svg_nested();
  tmv_tools_binary_encode();
  tmv_tools_binary_decode();

  tmv_tools_tmv_to_svg(flag_input, flag_output);

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
