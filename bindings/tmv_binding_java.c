/* tmv_binding_java.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Java binding for generating tmv treemap.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef TMV_BINDING_JAVA_H
/* #define TMV_BINDING_JAVA_H */

#include "../tmv.h"
#include "tmv_tokenizer.h"

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define TMV_BINDING_JAVA_INLINE inline
#define TMV_BINDING_JAVA_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_BINDING_JAVA_INLINE __inline__
#define TMV_BINDING_JAVA_API static
#elif defined(_MSC_VER)
#define TMV_BINDING_JAVA_INLINE __inline
#define TMV_BINDING_JAVA_API static
#else
#define TMV_BINDING_JAVA_INLINE
#define TMV_BINDING_JAVA_API static
#endif

typedef struct tmv_binding_java_user_data
{
    unsigned long file_size;
    unsigned long loc;
    unsigned long if_density;
    unsigned long count_new;
    unsigned long nesting_depth;

} tmv_binding_java_user_data;

#include "stdio.h"

void tmv_binding_java_print_token(tmv_tokenizer_token t)
{
    const char *type_str = "";
    switch (t.type)
    {
    case TOKEN_IDENTIFIER:
        type_str = "IDENT";
        break;
    case TOKEN_NUMBER:
        type_str = "NUMBER";
        break;
    case TOKEN_SYMBOL:
        type_str = "SYMBOL";
        break;
    case TOKEN_EOF:
        type_str = "EOF";
        break;
    default:
        type_str = "UNKNOWN";
        break;
    }

    printf("%-12s : %.*s\n", type_str, (int)t.len, t.start);
}

TMV_BINDING_JAVA_API TMV_BINDING_JAVA_INLINE void tmv_binding_java_map(tmv_treemap_item *result, char *java_source_file, unsigned long java_source_file_size)
{
    unsigned long depth = 0;

    tmv_binding_java_user_data *data = result->user_data;

    tmv_tokenizer tz;
    tmv_tokenizer_token t;

    tz.src = java_source_file;
    tz.len = java_source_file_size;
    tz.pos = 0;

    data->file_size = java_source_file_size;

    do
    {
        t = next_token(&tz);

        if (t.type == TOKEN_SYMBOL && t.len == 1)
        {
            if (t.start[0] == '{')
            {
                depth++;
                if (depth > data->nesting_depth)
                {
                    data->nesting_depth = depth;
                }
            }
            else if (t.start[0] == '}')
            {
                if (depth > 0)
                {
                    depth--;
                }
            }
        }

        if (t.type == TOKEN_IDENTIFIER && t.len == 2 && t.start[0] == 'i' && t.start[1] == 'f')
        {
            data->if_density++;
        }

        if (t.type == TOKEN_IDENTIFIER && t.len == 3 && t.start[0] == 'n' && t.start[1] == 'e' && t.start[2] == 'w')
        {
            data->count_new++;
        }

        tmv_binding_java_print_token(t);

    } while (t.type != TOKEN_EOF);
}

#endif /* TMV_BINDING_JAVA_H */

int main(void)
{
    char *java_source_file = "import java.io.*; public class HelloWorld {\n\npublic static void main(String[] args) {\n \tif(a==b) {new System.out.println(\"Hello World!\");\n}}\n\n}\n// End\n";
    unsigned long java_source_file_size = 151;

    tmv_binding_java_user_data data = {0};
    tmv_treemap_item result = {0};

    tmv_binding_java_user_data *p_data;

    result.user_data = (void *)&data;
    tmv_binding_java_map(&result, java_source_file, java_source_file_size);

    p_data = (tmv_binding_java_user_data *)result.user_data;

    printf("[data]     file_size: %lu\n", p_data->file_size);
    printf("[data]           loc: %lu\n", p_data->loc);
    printf("[data]    if_density: %lu\n", p_data->if_density);
    printf("[data]     count_new: %lu\n", p_data->count_new);
    printf("[data] nesting_depth: %lu\n", p_data->nesting_depth);

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
