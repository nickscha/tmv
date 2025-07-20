#ifndef TMV_TOOLS_H
#define TMV_TOOLS_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define TMV_TOOLS_INLINE inline
#define TMV_TOOLS_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_TOOLS_INLINE __inline__
#define TMV_TOOLS_API static
#elif defined(_MSC_VER)
#define TMV_TOOLS_INLINE __inline
#define TMV_TOOLS_API static
#else
#define TMV_TOOLS_INLINE
#define TMV_TOOLS_API static
#endif

#include "../tmv.h"             /* The squarified treemap algoryhtm */
#include "../tmv_platform_io.h" /* Platform specific IO functions   */
#include "deps/vgg.h"           /* The SVG generation library       */
#include "deps/test.h"          /* Assertions and printf            */

/* Printing functions */
TMV_TOOLS_API TMV_TOOLS_INLINE void tmv_tools_print_model(tmv_model *model, tmv_rect area)
{
    printf("##############################################\n");
    printf("# TMV Model Information                      #\n");
    printf("##############################################\n");
    printf("[area]                    id: %16lu\n", area.id);
    printf("[area]                     x: %16f\n", area.x);
    printf("[area]                     y: %16f\n", area.y);
    printf("[area]                 width: %16f\n", area.width);
    printf("[area]                height: %16f\n", area.height);
    printf("\n");
    printf("[stats]           weigth_min: %16f\n", model->stats.weigth_min);
    printf("[stats]           weigth_max: %16f\n", model->stats.weigth_max);
    printf("[stats]           weigth_sum: %16f\n", model->stats.weigth_sum);
    printf("[stats]                count: %16lu\n", model->stats.count);
    printf("\n");
    printf("[model]          items_count: %16lu\n", model->items_count);
    printf("[model] items_user_data_size: %16lu\n", model->items_user_data_size);
    printf("[model]          rects_count: %16lu\n", model->rects_count);
    printf("##############################################\n");
    printf("\n");
}

TMV_TOOLS_API TMV_TOOLS_INLINE void tmv_tools_print_items(tmv_model *model)
{
    unsigned long i;

    for (i = 0; i < model->items_count; ++i)
    {
        tmv_item item = model->items[i];

        printf(
            "[item][%4lu] id: %5lu, parent_id: %5li, weight: %12f, children_count: %5lu, children_offset_index: %5lu\n",
            i,
            item.id,
            item.parent_id,
            item.weight,
            item.children_count,
            item.children_offset_index);
    }
    printf("\n");
}

TMV_TOOLS_API TMV_TOOLS_INLINE void tmv_tools_print_rects(tmv_model *model)
{
    unsigned long i;
    for (i = 0; i < model->rects_count; ++i)
    {
        tmv_rect rect = model->rects[i];
        printf("[rect][%4lu] id: %5li, x: %12f, y: %12f, width: %12f, height: %12f\n", i, rect.id, rect.x, rect.y, rect.width, rect.height);
    }
    printf("\n");
}

/* The default color range of the treemap view */
static vgg_color color_start = {144, 224, 239}; /* Start (light teal): 144, 224, 239 */
static vgg_color color_end = {255, 85, 0};      /*  End (dark orange): 255,  85,   0 */

TMV_TOOLS_API TMV_TOOLS_INLINE double tmv_tools_ll_to_double(unsigned long lo, unsigned long hi)
{
    /* Compute (hi << 32) + lo using 16-bit shifts to avoid UB or optimizer tricks */
    double result;

    double hi_part = ((double)(hi & 0xFFFF)) * 65536.0 * 65536.0;
    hi_part += ((double)((hi >> 16) & 0xFFFF)) * 65536.0 * 65536.0 * 65536.0;

    result = hi_part + (double)lo;

    return result;
}

TMV_TOOLS_API TMV_TOOLS_INLINE void tmv_tools_write_to_svg(char *filename, unsigned char *vgg_buffer, unsigned long vgg_buffer_capacity, tmv_model *model, tmv_rect *area)
{
    unsigned long i;

    vgg_svg_writer w;

    /* Zero vgg_buffer */
    for (i = 0; i < vgg_buffer_capacity; ++i)
    {
        vgg_buffer[i] = 0;
    }

    w.buffer = vgg_buffer;
    w.capacity = (int)vgg_buffer_capacity;
    w.length = 0;

    vgg_svg_start(&w, "tmvsvg", area->width, area->height);

    for (i = 0; i < model->rects_count; ++i)
    {
        tmv_rect rect = model->rects[i];
        tmv_item *item = tmv_find_item_by_id(model->items, model->items_count, rect.id);

        char d1_buffer[32];
        char d2_buffer[32];
        char d3_buffer[32];
        char d4_buffer[32];
        char d5_buffer[32];

        vgg_rect r = {0};
        vgg_data_field data_fields[5];
        data_fields[0] = vgg_data_field_create_double("rect-x", rect.x, 6, d1_buffer);
        data_fields[1] = vgg_data_field_create_double("rect-y", rect.y, 6, d2_buffer);
        data_fields[2] = vgg_data_field_create_double("rect-width", rect.width, 6, d3_buffer);
        data_fields[3] = vgg_data_field_create_double("rect-height", rect.height, 6, d4_buffer);
        data_fields[4] = vgg_data_field_create_double("weight", item->weight, 3, d5_buffer);

        r.header.id = (unsigned long)rect.id;
        r.header.type = VGG_TYPE_RECT;
        r.header.color_fill = vgg_color_map_linear(item->weight, model->stats.weigth_min, model->stats.weigth_max, color_start, color_end);
        r.header.data_fields = data_fields;
        r.header.data_fields_count = TMV_ARRAY_SIZE(data_fields);
        r.x = rect.x;
        r.y = rect.y;
        r.width = rect.width;
        r.height = rect.height;

        vgg_svg_element_add(&w, (vgg_header *)&r);
    }

    vgg_svg_end(&w);

    tmv_platform_write(filename, w.buffer, (unsigned long)w.length);
}

TMV_TOOLS_API TMV_TOOLS_INLINE int tmv_tools_scan_files(
    const char *path,
    tmv_item *items_buffer,
    unsigned long *items_count,
    unsigned long items_capacity,
    long parent_id)
{
    char search_path[TMV_PLATFORM_WIN32_MAX_PATH];
    TMV_PLATFORM_WIN32_FIND_DATAA ffd;
    void *hFind = TMV_PLATFORM_WIN32_INVALID_HANDLE;
    int len = 0;

    if (*items_count >= items_capacity)
    {
        return 1;
    }

    while (path[len] != '\0' && len < TMV_PLATFORM_WIN32_MAX_PATH - 3)
    {
        search_path[len] = path[len];
        ++len;
    }
    if (len > 0 && search_path[len - 1] != '\\')
    {
        search_path[len++] = '\\';
    }
    search_path[len++] = '*';
    search_path[len] = '\0';

    hFind = FindFirstFileA(search_path, &ffd);
    if (hFind == TMV_PLATFORM_WIN32_INVALID_HANDLE)
    {
        return 1;
    }

    do
    {
        tmv_item *item;

        char full_path[TMV_PLATFORM_WIN32_MAX_PATH];
        int len = 0, i = 0;

        if (ffd.cFileName[0] == '.' &&
            (ffd.cFileName[1] == '\0' || (ffd.cFileName[1] == '.' && ffd.cFileName[2] == '\0')))
        {
            continue;
        }

        while (path[len] != '\0' && len < TMV_PLATFORM_WIN32_MAX_PATH - 1)
        {
            full_path[len] = path[len];
            ++len;
        }

        if (len > 0 && full_path[len - 1] != '\\')
        {
            full_path[len++] = '\\';
        }

        while (ffd.cFileName[i] != '\0' && len < TMV_PLATFORM_WIN32_MAX_PATH - 1)
        {
            full_path[len++] = ffd.cFileName[i++];
        }
        full_path[len] = '\0';

        if (*items_count >= items_capacity)
        {
            FindClose(hFind);
            return 1;
        }

        item = &items_buffer[*items_count];
        item->id = (long)(*items_count);
        item->parent_id = parent_id;
        item->weight = 0.0;
        item->children_count = 0;
        item->children_offset_index = 0;

        if (ffd.dwFileAttributes & TMV_PLATFORM_WIN32_FILE_ATTRIBUTE_DIRECTORY)
        {
            unsigned long before;
            unsigned long after;
            double total;
            unsigned long j;

            unsigned long dir_index = *items_count;
            item->weight = 0.0;

            ++(*items_count);

            before = *items_count;

            tmv_tools_scan_files(full_path, items_buffer, items_count, items_capacity, (long)dir_index);

            after = *items_count;
            total = 0.0;

            for (j = before; j < after; ++j)
            {
                if (items_buffer[j].parent_id == (long)dir_index)
                {
                    total += items_buffer[j].weight;
                }
            }
            items_buffer[dir_index].weight = (double)total;
        }
        else
        {
            double size = tmv_tools_ll_to_double(ffd.nFileSizeLow, ffd.nFileSizeHigh);

            item->weight = size;

            ++(*items_count);
        }

    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);

    return 0;
}

#endif /* TMV_TOOLS_H */