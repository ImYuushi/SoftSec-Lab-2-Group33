#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf jump_buffer;


// create a callback function for png_set_read_fn
void my_read_callback(png_structp png_ptr,
                      png_bytep outBytes,
                      png_size_t byteCountToRead)
{
    FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
    fread(outBytes, 1, byteCountToRead, fp);
}

#ifdef FUZZ_PROGRESSIVE
void on_info_callback(png_structp png_ptr, png_infop info_ptr) {
    // header has been parsed, nothing to do for fuzzing
    (void)png_ptr;
    (void)info_ptr;
}

void on_row_callback(png_structp png_ptr, png_bytep new_row,
                     png_uint_32 row_num, int pass) {
    // called for each decoded row, nothing to do for fuzzing
    (void)png_ptr;
    (void)new_row;
    (void)row_num;
    (void)pass;
}

void on_end_callback(png_structp png_ptr, png_infop info_ptr) {
    // end of image, nothing to do for fuzzing
    (void)png_ptr;
    (void)info_ptr;
}
#endif

int main(int argc, char *argv[]) {
    // Get the path into a variable
    if (argc <= 0) {
        return 0;
    }

#ifdef FUZZ_PERSISTENT
    while (__AFL_LOOP(10000)) {
#endif

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
#ifdef FUZZ_PERSISTENT
        continue;
#else
        return 0;
#endif
    }
    
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_uint_32 width, height;
    int bit_depth, color_type;
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    bit_depth = png_get_bit_depth(png, info);
    color_type = png_get_color_type(png, info);


    // Do the setjmp shenanigans. TlDR: bad input -> no Crash but return 
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return 0;
    }
    /* set up input source */


    /* read header and metadata chunks */
    #ifdef PNG_READ_INFO_SUPPORTED
    png_set_read_fn(png, fp, my_read_callback);
    png_read_info(png, info);
    #endif
    /* optionally apply transformations */

    // Here we probably want to be able to restrict WHAT we test, by either using macros or user input?
    #ifdef PNG_READ_IHDR_SUPPORTED
    int interlace_method;
    int compression_method;
    int filter_method;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type,
                &interlace_method, &compression_method, &filter_method);
    #endif

    #ifdef PNG_READ_PLTE_SUPPORTED
    png_get_PLTE(png, info, &palette, &num_palette);
    #endif

    #ifdef PNG_READ_tRNS_SUPPORTED
    png_bytep trans_alpha = NULL;
    int num_trans = 0;
    png_color_16p trans_color = NULL;
    png_get_tRNS(png, info, &trans_alpha, &num_trans, &trans_color);
    #endif




    #ifdef PNG_READ_EXPAND_SUPPORTED
    png_set_expand(png); /* palette -> RGB */
    #endif

    #ifdef PNG_READ_STRIP_16_SUPPORTED
    png_set_strip_16(png); /* 16-bit -> 8-bit */
    #endif

    #ifdef PNG_READ_GRAY_TO_RGB_SUPPORTED
    png_set_gray_to_rgb(png); /* grayscale -> RGB */
    #endif
    /* read pixel data */

    #ifdef FUZZ_READ_IMAGE
    png_size_t rowbytes = png_get_rowbytes(png, info);
    png_bytep *row_pointers =
    malloc(sizeof(png_bytep) * height);

    for (png_uint_32 y = 0; y < height; y++) {
        row_pointers[y] = malloc(rowbytes);
    }
    png_read_image(png, row_pointers);
    height = png_get_image_height(png, info);
    
   
    #endif

    #ifdef FUZZ_PROGRESSIVE
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    png_bytep buf = malloc(size);
    if (!buf) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return 0;
    }
    fread(buf, 1, size, fp);

    png_set_progressive_read_fn(png, NULL,
                                on_info_callback,
                                on_row_callback,
                                on_end_callback);

    png_process_data(png, info, buf, size);
    free(buf);
    #endif
    #ifdef FUZZ_INDUCE_BUG
    if (height == 0) {
        height = 1;
    }

    png_bytep *bug_rows =
        malloc(sizeof(png_bytep) * height);

    if (bug_rows) {

        volatile png_bytep sink;

        /*
        * Valid indices:
        *   0 .. height-1
        *
        * BUG:
        *   accesses bug_rows[height]
        */

        for (png_uint_32 y = 0; y <= height; y++) {
            sink = bug_rows[y];
        }

        free(bug_rows);
    }
    #endif
    #ifdef FUZZ_TRANSFORMS
    png_set_expand(png);
    png_set_strip_16(png);
    #endif
    /* read pixel data */


    /* read post-IDAT chunks */

    /* clean up */
    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);

#ifdef FUZZ_PERSISTENT
    } /* end __AFL_LOOP */
#endif
}