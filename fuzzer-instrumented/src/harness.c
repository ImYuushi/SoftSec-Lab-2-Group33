#include <png.h>
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



int main(int argc, char *argv[]) {
    // Get the path into a variable
    if (argc <= 0) {
        return 0;
    }
    
    // Read file into png_file from input, i. e. argv, simple (or at least get a pointer to it?)
    FILE *fp = fopen(argv[1], "rb");    
    // initialize png_struct or pointer to it?

    
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_uint_32 width, height;
    int bit_depth, color_type;
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    bit_depth = png_get_bit_depth(png, info);
    color_type = png_get_color_type(png, info);
    // Do the setjmp shenanigans. TlDR: bad input -> no Crash but return 
    if (setjmp(jump_buffer)) { // will be called by png in case of error. If no calls but just crash, no setjmp and afl logs it

        // play trashman
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
    #endif

    #ifdef FUZZ_PROGRESSIVE
    png_set_progressive_read_fn(png, NULL,
                            info_callback,
                            row_callback,
                            end_callback);
    png_process_data(png, info, buf, size);
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
}