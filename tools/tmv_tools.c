/* tmv.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) squarified tree map viewer (TMV).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "tmv_tools.h" /* Developer api for tmv tools */
#include "deps/clp.h"  /* Command Line Parser         SS*/

typedef struct tmv_tools_memory
{
  unsigned char *vgg_buffer;
  unsigned long vgg_buffer_size;
  unsigned long vgg_buffer_capacity;

  unsigned char *io_buffer;
  unsigned long io_buffer_size;
  unsigned long io_buffer_capacity;

  tmv_item *items_buffer;
  unsigned long items_buffer_size;
  unsigned long items_buffer_capacity;

  tmv_rect *rects_buffer;
  unsigned long rects_buffer_size;
  unsigned long rects_buffer_capacity;

} tmv_tools_memory;

void tmv_tools_memzero(tmv_tools_memory *memory)
{
  memory->vgg_buffer_size = 0;
  memory->io_buffer_size = 0;
  memory->items_buffer_size = 0;
  memory->rects_buffer_size = 0;
}

void tmv_tools_files_to_tmv(tmv_tools_memory *memory, char *output_tmv_file, char *path, tmv_rect area)
{
  char *exts[] = {".c", ".h"};

  tmv_model model = {0};

  tmv_tools_scan_files(
      path, memory->items_buffer,
      &memory->items_buffer_size,
      memory->items_buffer_capacity,
      -1,
      exts,
      0);

  model.items = memory->items_buffer;
  model.items_count = memory->items_buffer_size;
  model.rects = memory->rects_buffer;
  model.rects_count = memory->rects_buffer_size;

  /* Build squarified recursive treemap view */
  tmv_squarify(
      &model,
      area);

  tmv_tools_print_model(&model, area);

  /* (2) Decode tmv file to tmv_model and tmv_rect area */
  tmv_binary_encode(memory->io_buffer, memory->io_buffer_capacity, &memory->io_buffer_size, &model, area);

  tmv_platform_write(output_tmv_file, memory->io_buffer, memory->io_buffer_size);
}

void tmv_tools_tmv_to_svg(tmv_tools_memory *memory, char *input_tmv_file, char *output_svg_file)
{

  tmv_model model = {0};
  tmv_rect area = {0};

  /* (1) Read the tmv file */
  tmv_platform_read(input_tmv_file, memory->io_buffer, memory->io_buffer_capacity, &memory->io_buffer_size);

  /* (2) Decode tmv file to tmv_model and tmv_rect area */
  tmv_binary_decode(memory->io_buffer, memory->io_buffer_size, &model, &area);

  /* (3) Write the tmv_model as SVG */
  tmv_tools_write_to_svg(output_svg_file, memory->vgg_buffer, memory->vgg_buffer_capacity, &model, &area);
}

#define TMV_TOOLS_FLAGS 3
#include <stdlib.h>

int main(int argc, char **argv)
{
  unsigned long memory_vgg_capacity = 1024 * 1024 * 256;            /* 64 MB for SVG Buffer */
  unsigned long memory_io_capacity = 1024 * 1024 * 32;             /* 32 MB for files      */
  unsigned long memory_items_capacity = sizeof(tmv_item) * 200000; /* tmv_items            */
  unsigned long memory_rects_capacity = sizeof(tmv_rect) * 200000; /* tmv_rects            */
  tmv_rect area = {0, 0.0, 0.0, 800.0, 300.0};

  tmv_tools_memory memory = {0};

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

  /* Initialize memory buffers */
  memory.vgg_buffer = malloc(memory_vgg_capacity);
  memory.vgg_buffer_capacity = memory_vgg_capacity;
  memory.io_buffer = malloc(memory_io_capacity);
  memory.io_buffer_capacity = memory_io_capacity;
  memory.items_buffer = malloc(memory_items_capacity);
  memory.items_buffer_capacity = memory_items_capacity;
  memory.rects_buffer = malloc(memory_rects_capacity);
  memory.rects_buffer_capacity = memory_rects_capacity;

  tmv_tools_tmv_to_svg(&memory, flag_input, flag_output);
  tmv_tools_memzero(&memory);

  tmv_tools_files_to_tmv(&memory, "test.tmv", "..", area);
  tmv_tools_memzero(&memory);

  tmv_tools_tmv_to_svg(&memory, "test.tmv", "test.svg");
  tmv_tools_memzero(&memory);

  free(memory.vgg_buffer);
  free(memory.io_buffer);
  free(memory.items_buffer);
  free(memory.rects_buffer);

  printf("finished\n");

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
