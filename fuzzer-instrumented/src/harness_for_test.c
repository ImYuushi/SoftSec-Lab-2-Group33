#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define MAX_INPUT_SIZE (1024 * 1024)
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096

typedef struct {
    const unsigned char *data;
    size_t size;
    size_t offset;
} png_mem_t;

static void read_callback(png_structp png_ptr, png_bytep out, png_size_t count) {
    png_mem_t *mem = (png_mem_t *)png_get_io_ptr(png_ptr);

    if (!mem || mem->offset > mem->size || count > mem->size - mem->offset) {
        png_error(png_ptr, "read past end of input");
        return;
    }

    memcpy(out, mem->data + mem->offset, count);
    mem->offset += count;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }

    long file_size = ftell(fp);
    if (file_size <= 0 || file_size > MAX_INPUT_SIZE) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    unsigned char *data = malloc((size_t)file_size);
    if (!data) {
        fclose(fp);
        return 0;
    }

    size_t nread = fread(data, 1, (size_t)file_size, fp);
    fclose(fp);

    if (nread != (size_t)file_size) {
        free(data);
        return 0;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        free(data);
        return 0;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        free(data);
        return 0;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        free(data);
        return 0;
    }

    png_mem_t mem = {
        .data = data,
        .size = (size_t)file_size,
        .offset = 0
    };

    png_set_read_fn(png, &mem, read_callback);

    png_read_info(png, info);

    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);

    if (width == 0 || height == 0 || width > MAX_WIDTH || height > MAX_HEIGHT) {
        png_destroy_read_struct(&png, &info, NULL);
        free(data);
        return 0;
    }

    int color_type = png_get_color_type(png, info);
    int bit_depth = png_get_bit_depth(png, info);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }

    if (bit_depth == 16) {
        png_set_strip_16(png);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    png_read_update_info(png, info);

    png_size_t rowbytes = png_get_rowbytes(png, info);
    if (rowbytes == 0 || rowbytes > MAX_WIDTH * 8) {
        png_destroy_read_struct(&png, &info, NULL);
        free(data);
        return 0;
    }

    png_bytep *rows = calloc(height, sizeof(png_bytep));
    if (!rows) {
        png_destroy_read_struct(&png, &info, NULL);
        free(data);
        return 0;
    }

    for (png_uint_32 y = 0; y < height; y++) {
        rows[y] = malloc(rowbytes);
        if (!rows[y]) {
            for (png_uint_32 i = 0; i < y; i++) {
                free(rows[i]);
            }
            free(rows);
            png_destroy_read_struct(&png, &info, NULL);
            free(data);
            return 0;
        }
    }

    png_read_image(png, rows);
    png_read_end(png, NULL);

    for (png_uint_32 y = 0; y < height; y++) {
        free(rows[y]);
    }

    free(rows);
    png_destroy_read_struct(&png, &info, NULL);
    free(data);

    return 0;
}
