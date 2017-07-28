/* jpg.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */
/* JPG code originally by Sam Lantinga, slouken@libsdl.org */

#include <stdio.h>
#include <string.h>
#include <jpeglib.h>
#include <SDL_video.h>
#include <SDL_error.h>
#include <SDL_rwops.h>

#define INPUT_BUFFER_SIZE	4096
typedef struct {
  struct jpeg_source_mgr pub;

  SDL_RWops *ctx;
  Uint8 buffer[INPUT_BUFFER_SIZE];
} my_source_mgr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source(j_decompress_ptr cinfo)
{
  /* We don't actually need to do anything */
  return;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 */
static int fill_input_buffer(j_decompress_ptr cinfo)
{
  my_source_mgr *src = (my_source_mgr *) cinfo->src;
  int nbytes;

  nbytes = SDL_RWread(src->ctx, src->buffer, 1, INPUT_BUFFER_SIZE);
  if (nbytes <= 0) {
    /* Insert a fake EOI marker */
    src->buffer[0] = (Uint8) 0xFF;
    src->buffer[1] = (Uint8) JPEG_EOI;
    nbytes = 2;
  }
  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;

  return TRUE;
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  my_source_mgr *src = (my_source_mgr *) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) src->pub.fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never
       * return FALSE, so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.
 */
static void term_source(j_decompress_ptr cinfo)
{
  /* We don't actually need to do anything */
  return;
}

/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
static void jpeg_SDL_RW_src(j_decompress_ptr cinfo, SDL_RWops * ctx)
{
  my_source_mgr *src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) {			 /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(my_source_mgr));
    src = (my_source_mgr *) cinfo->src;
  }

  src = (my_source_mgr *) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;	/* use default method */
  src->pub.term_source = term_source;
  src->ctx = ctx;
  src->pub.bytes_in_buffer = 0;			 /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL;		 /* until buffer loaded */
}

/* Load a JPEG type image */
SDL_Surface *loadJPG(Uint8 * data, Uint32 * memcounter)
{
  struct jpeg_error_mgr errmgr;
  struct jpeg_decompress_struct cinfo;
  JSAMPROW rowptr[1];
  SDL_Surface *surface;
  SDL_RWops *src = NULL;
  Uint32 size;

  memcpy(&size, data, sizeof(Uint32));
  if (memcounter)
    *memcounter += size + sizeof(Uint32);
  data += sizeof(Uint32);
  src = SDL_RWFromMem(data, size);

  /* Create a decompression structure and load the JPEG header */
  cinfo.err = jpeg_std_error(&errmgr);
  jpeg_create_decompress(&cinfo);
  jpeg_SDL_RW_src(&cinfo, src);
  jpeg_read_header(&cinfo, TRUE);

  /* Set 24-bit RGB output */
  cinfo.out_color_space = JCS_RGB;
  cinfo.quantize_colors = FALSE;
  jpeg_calc_output_dimensions(&cinfo);

  /* Allocate an output surface to hold the image */
  surface = SDL_AllocSurface(SDL_SWSURFACE,
			     cinfo.output_width, cinfo.output_height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			     0x0000FF, 0x00FF00, 0xFF0000,
#else
			     0xFF0000, 0x00FF00, 0x0000FF,
#endif
			     0);
  if (surface == NULL) {
    SDL_SetError("Out of memory");
    goto done;
  }

  /* Decompress the image */
  jpeg_start_decompress(&cinfo);
  while (cinfo.output_scanline < cinfo.output_height) {
    rowptr[0] = (JSAMPROW) (Uint8 *) surface->pixels +
      cinfo.output_scanline * surface->pitch;
    jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
  }
  jpeg_finish_decompress(&cinfo);

  /* Clean up and return */
done:
  if (src)
    SDL_FreeRW(src);
  jpeg_destroy_decompress(&cinfo);
  return (surface);
}
