#ifndef PNG_FUZZ_H
#define PNG_FUZZ_H

#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Fuzzing target flags ──────────────────────────────────────────────────
 * Pass exactly one of these via -D at compile time to select the target.
 * The build script generates a separate binary for every flag.
 *
 *   FUZZ_READ_IMAGE      – decompress and allocate all pixel rows
 *   FUZZ_PROGRESSIVE     – feed data through the progressive decoder
 *   FUZZ_TRANSFORMS      – apply expand + strip-16 transforms
 *
 * Chunk-level flags (always compiled in when libpng supports them):
 *   PNG_READ_IHDR_SUPPORTED
 *   PNG_READ_PLTE_SUPPORTED
 *   PNG_READ_tRNS_SUPPORTED
 *   PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
 *   PNG_READ_EXPAND_SUPPORTED
 *   PNG_READ_STRIP_16_SUPPORTED
 *   PNG_READ_GRAY_TO_RGB_SUPPORTED
 * ────────────────────────────────────────────────────────────────────────── */

/* Jump buffer used by the libpng error handler */
extern jmp_buf jump_buffer;

/* Custom read callback – wraps fread for png_set_read_fn */
void my_read_callback(png_structp png_ptr,
                      png_bytep   outBytes,
                      png_size_t  byteCountToRead);

/* State struct used by the progressive decoder path */
typedef struct {
    png_structp png;
    png_infop   info;
} progressive_state_t;

#ifdef FUZZ_PROGRESSIVE
void on_info_callback(png_structp png_ptr, png_infop   info_ptr);
void on_row_callback (png_structp png_ptr, png_bytep   new_row,
                      png_uint_32 row_num, int         pass);
void on_end_callback (png_structp png_ptr, png_infop   info_ptr);
#endif

#endif /* PNG_FUZZ_H */