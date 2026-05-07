#include <png.h>
#include <setjmp.h>

jmp_buf jump_buffer;

int main(int argc, char *argv[]) {
    // Get the path into a variable
    if (argc <= 0) {
        return 0;
    }
    
    // Read file into png_file from input, i. e. argv, simple (or at least get a pointer to it?)
    FILE *fp = fopen(argv[1], "rb");    
    // initialize png_struct or pointer to it?

    
    png_structp png = png_create_read_struct();
    png_infop info = png_create_info_struct(png);

    // Do the setjmp shenanigans. TlDR: bad input -> no Crash but return 
    if (setjmp(jump_buffer)) { // will be called by png in case of error. If no calls but just crash, no setjmp and afl logs it

        // play trashman
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        
        return 0;
    }
    /* set up input source */

    // TODO: Figure out if we need callback function or not. If so, implement it
    png_set_read_fn(png, fp, my_read_callback);
    /* read header and metadata chunks */
    png_read_info(png, info);
    /* optionally apply transformations */

    // Here we probably want to be able to restrict WHAT we test, by either using macros or user input?

    // png_set_expand(png); /* palette -> RGB */
    // png_set_strip_16(png); /* 16-bit -> 8-bit */
    // png_set_gray_to_rgb(png); /* grayscale -> RGB */
    png_read_update_info(png, info);
    /* read pixel data */

    // TODO: initialize row pointers properly. Figure out its type because we are not writing python. Then fix it to the start of each row of the png pixels
    void* row_pointers = NULL;
    png_read_image(png, row_pointers);
    /* read post-IDAT chunks */
    png_read_end(png, NULL);
    /* clean up */
    png_destroy_read_struct(&png, &info, NULL);
}