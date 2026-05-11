#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const unsigned char *data;
    size_t size;
    size_t offset;
} png_buffer_t;

void png_memory_read(png_structp png_ptr, png_bytep out, png_size_t count) {
    png_buffer_t *buf = (png_buffer_t *)png_get_io_ptr(png_ptr);
    if (buf->offset + count > buf->size) 
        png_error(png_ptr, "read error");
    memcpy(out, buf->data + buf->offset, count);
    buf->offset += count;
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buf_data = malloc(size);
    fread(buf_data, 1, size, f);
    fclose(f);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) { free(buf_data); return 1; }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) { png_destroy_read_struct(&png_ptr, NULL, NULL); free(buf_data); return 1; }

    if (setjmp(png_jmpbuf(png_ptr))) { png_destroy_read_struct(&png_ptr, &info_ptr, NULL); free(buf_data); return 1; }

    png_buffer_t buffer = { buf_data, (size_t)size, 0 };
    png_set_read_fn(png_ptr, &buffer, png_memory_read);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(buf_data);
    return 0;
}